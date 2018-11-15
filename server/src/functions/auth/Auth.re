[@bs.module "jsonwebtoken"] external jwtVerify : (string, string) => 'a = "verify";
[@bs.module "jsonwebtoken"] external jwtSign : (Js.Json.t, string, Js.Json.t, (Js.null(exn), string) => unit) => unit = "sign";
external encode : 'a => Js.Json.t = "%identity";
external decode : Js.Json.t => 'a = "%identity";

exception Return(Js.Json.t, string);

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

let sign : (payload, string, Js.Json.t) => Js.Promise.t(string) = (payload, secret, option) => {
  Js.Promise.make((~resolve, ~reject) => {
    jwtSign(payload |> encode, secret, option, (err, token) => {
      if (err != Js.null) {
        reject(. err |> Js.Null.getExn);
      } else {
        resolve(. token);
      }
    });
  });
}

/* Is there a way to catch undefined authorizationContext as it is with type-safety? */
let authorize : (Js.Dict.t(Js.Json.t), 'a, (. Js.Json.t, string) => unit) => Js.Promise.t(unit) = (event, _context, callback) => {
  let buildPolicyDoc = (principalId, effect, resource, context) => {
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

  let verify = (token, methodArn) => {
    let decoded : payload = jwtVerify(token, Node.Process.process##env |> Js.Dict.unsafeGet(_, "jwt_public"));

    /* skip user scope check now */
    let isAllowed = true;

    let effect = if (isAllowed) { "Allow" } else { "Deny" };
    let userId = decoded##user_id;
    let authorizerContext = [%bs.obj {
      user: {
        userId: userId,
      },
    }];
    buildPolicyDoc(userId, effect, methodArn, authorizerContext);
  };

  let run = () => {
    let token = event
      |> Js.Dict.get(_, "authorizationToken")
      |> Js.Option.andThen((. v) => Js.Json.decodeString(v));
    let methodArn = event
      |> Js.Dict.get(_, "methodArn")
      |> Js.Option.andThen((. v) => Js.Json.decodeString(v));

    let token = switch token {
      | Some(token) => token
      | None => raise(Return(
        "Unauthorized" |> Js.Json.string,
        buildPolicyDoc("user", "Deny", methodArn |> Js.Option.getExn, Js.null)
        |> Js.Json.stringify
      ))
    };

    let doc = verify(
      token,
      methodArn |> Js.Option.getExn,
    );

    raise(Return(Js.Json.null, doc |> Js.Json.stringify));
  };

  try (run()) {
    | Return(x,y) => callback(. x,y) |> Js.Promise.resolve
  }
};

let clientID = "568268689599-r3s51pdntp56iu6l1amus3eq29johlh3.apps.googleusercontent.com";

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

let signUp = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let body : Js.t({
    .
    token: string,
    user_name: string,
  }) = event
    |> bodyGet
    |> Js.Option.getExn
    |> Js.Json.parseExn
    |> decode;
  
  idpVerify(body##token)
  |> Js.Promise.then_(idpId => {
    DB.User.create(
      {
        "google": idpId,
      },
      body##user_name,
    )
    |> Js.Promise.then_(userId => {
      Js.Promise.all([|
        DB.User.createIdp("google", idpId, userId),
        DB.User.createUsername(userId, body##user_name),
      |])
      |> Js.Promise.then_(result => {
        Result.make(
          ~statusCode=200,
          ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
          ~body="{}",
          ()
        )
        |> Js.Promise.resolve;
      });
    });
  });
};

let signIn = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let body : Js.t({
    .
    token: string,
  }) = event
    |> bodyGet
    |> Js.Option.getExn
    |> Js.Json.parseExn
    |> decode;

  idpVerify(body##token)
  |> Js.Promise.then_((idpId : string) => {
    DB.User.findViaIdp("google", idpId)
    |> Js.Promise.then_((result : DB.QueryResult.one(DB.Idp.t)) => {
      DB.User.findById(result.item##owned_by)
      |> Js.Promise.then_((result : DB.QueryResult.one(DB.User.t)) => {
        let token = sign(
          [%bs.obj {
            user_id: result.item##id,
            user_name: result.item##user_name,
            idp: {
              google: idpId,
            }
          }],
          Node.Process.process##env |> Js.Dict.unsafeGet(_, "jwt_private"),
          [%bs.obj {
            algorithm: "RS256",
          }] |> encode
        );

        token
        |> Js.Promise.then_(token => {
          Result.make(
            ~statusCode=200,
            ~headers=Js.Dict.fromArray([|
              ("Access-Control-Allow-Origin", Js.Json.string("*")),
              ("Access-Control-Allow-Credentials", Js.Json.boolean(true)),
              ("Set-Cookie", Js.Json.string({j|token=$token; Max-Age=7200;|j}))
            |]),
            ~body=Js.Json.null |> Js.Json.stringify,
            ()
          )
          |> Js.Promise.resolve;
        })
        |> Js.Promise.resolve;
      })
      |> Js.Promise.resolve;
    })
    |> Js.Promise.resolve;
  });
};