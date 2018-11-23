[@bs.module "jsonwebtoken"] external jwtVerify : (string, string, Js.Json.t) => 'a = "verify";
[@bs.module "jsonwebtoken"] external jwtSign : (Js.Json.t, string, Js.Json.t) => string = "sign";
external encode : 'a => Js.Json.t = "%identity";
external decode : Js.Json.t => 'a = "%identity";

exception Return(Js.Json.t, Js.Json.t);

/* jwt payload */
type payload = Js.t({
  .
  user_id: string,
  user_name: string,
  idp: Js.t({
    .
    google: string,
  }),
});

/* Is there a way to catch undefined authorizationContext as it is with type-safety? */
let authorize : (Js.Dict.t(Js.Json.t), 'a, (. Js.Json.t, Js.Json.t) => unit) => Js.Promise.t(unit) = (event, _context, callback) => {
  let generatePolicy = (principalId, effect, resource, context) => {
    {
      "principalId": principalId,
      "policyDocument": {
        "Version": "2012-10-17",
        "Statement": [|
          {
            "Action": "execute-api:Invoke",
            "Effect": effect,
            "Resource": resource,
          }
        |]
      },
      "context": context,
    } |> encode;
  };

  let run = () => {
    let token = event
      |> Js.Dict.get(_, "authorizationToken")
      |> Js.Option.andThen((. v) => Js.Json.decodeString(v))
      |> Js.Option.map((. v) => v |> Js.String.split("Bearer ") |> x => x[1]);

    let methodArn = event
      |> Js.Dict.get(_, "methodArn")
      |> Js.Option.andThen((. v) => Js.Json.decodeString(v))
      |> Js.Option.getExn;

    let token = switch token {
      | Some(token) => token
      | None => raise(Return(
        "Unauthorized" |> Js.Json.string,
        generatePolicy("user", "Deny", methodArn, Js.null)
      ))
    };

    let decoded : payload = try (jwtVerify(
      token,
      Node.Process.process##env |> Js.Dict.unsafeGet(_, "jwt_public"),
      {
        "algorithms": [| "ES256" |]
      } |> encode
    )) {
      | Js.Exn.Error(_) => raise(Return("Unauthorized" |> Js.Json.string, generatePolicy("user", "Deny", methodArn, Js.null)))
    };

    /* skip user scope check now */
    let isAllowed = true;

    let effect = if (isAllowed) { "Allow" } else { "Deny" };
    let userId = decoded##user_id;
    let authorizerContext = [%bs.obj {
      userId: userId,
    }];

    raise(Return(
      Js.Json.null,
      generatePolicy(userId, effect, methodArn, authorizerContext)
    ));
  };

  try (run()) {
    | Return(x,y) => callback(. x,y) |> Js.Promise.resolve
  }
};

let clientID : string = Node.Process.process##env |> Js.Dict.unsafeGet(_, "google_clientid");

type gClient;
let gclient : gClient = [%raw {|
  (() => {
    const { OAuth2Client } = require('google-auth-library');
    return new OAuth2Client(clientID);
  })()
|}];
[@bs.send] external gverifyIdToken : (gClient, Js.t({
  .
  idToken: string,
  audience: string,
})) => Js.Promise.t(Js.t({ . [@bs.meth] getPayload : unit => Js.Json.t })) = "verifyIdToken";

let idpVerify : (string) => Js.Promise.t(string) = token => {
  gverifyIdToken(gclient, [%bs.obj {
    idToken: token,
    audience: clientID,
  }])
  |> Js.Promise.then_(result => {
    result##getPayload()
    |> Js.Json.decodeObject |> Js.Option.getExn
    |> Js.Dict.unsafeGet(_, "sub")
    |> Js.Json.decodeString |> Js.Option.getExn
    |> Js.Promise.resolve;
  });
};

let userNameAvailable = Controller.wrapper((event) => {
  let userName : string = event##pathParameters
    |> Js.Dict.unsafeGet(_, "user_name");

  DB.User.getByName(userName)
  |> Js.Promise.then_(result => {
    Js.log(result);
    Js.log(result |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.get(_, "Item"));

    (if (result |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.get(_, "Item") |> Js.Option.isNone) {
      "true"
    } else {
      "false"
    })
    |> Js.Json.string
    |> Js.Promise.resolve;
  });
});

let signUp = Controller.wrapper((event) => {
  let body : Js.t({
    .
    token: string,
    user_name: string,
  }) = event##body
    |> Js.Json.parseExn
    |> decode;

  idpVerify(body##token)
  |> Js.Promise.then_(idpId => {
    Js.Promise.all2((
      DB.User.getByName(body##user_name),
      DB.User.findViaIdp("google", idpId)
      |> Js.Promise.then_(_ => true |> Js.Promise.resolve)
      |> Js.Promise.catch(_ => false |> Js.Promise.resolve),
    ))
    |> Js.Promise.then_(((result_name, result_idp)) => {
      if (result_name |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.get(_, "Item") |> Js.Option.isSome || result_idp) {
        "Validation Error"
        |> Js.Json.string
        |> Js.Promise.resolve;
      } else {
        DB.User.create(
          {
            "google": idpId,
          },
          body##user_name,
        )
        |> Js.Promise.then_(userId => {
          Js.Promise.all([|
            DB.User.createIdp("google", idpId, userId),
            DB.User.createNameAlias(body##user_name, userId),
          |])
          |> Js.Promise.then_(_ => {
            { "result": true }
            |> RawJson.encode
            |> Js.Promise.resolve;
          });
        });
      }
    });
  });
});

let signIn = Controller.wrapper((event) => {
  let body : Js.t({
    .
    token: string,
  }) = event##body
    |> Js.Json.parseExn
    |> decode;

  idpVerify(body##token)
  |> Js.Promise.then_((idpId : string) => {
    DB.User.findViaIdp("google", idpId)
    |> Js.Promise.then_((result : DB.QueryResult.one(DB.Idp.t)) => {
      DB.User.findById(result.item##owned_by)
      |> Js.Promise.then_((result : DB.QueryResult.one(DB.User.t)) => {
        let token = jwtSign(
          [%bs.obj {
            user_id: result.item##id,
            user_name: result.item##user_name,
            idp: {
              google: idpId,
            }
          }] |> encode,
          Node.Process.process##env |> Js.Dict.unsafeGet(_, "jwt_private"),
          [%bs.obj {
            algorithm: "ES256",
          }] |> encode
        );

        { "token": token }
        |> RawJson.encode
        |> Js.Promise.resolve;
      });
    });
  });
});
