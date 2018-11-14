[@bs.module "jsonwebtoken"] external jwtVerify : (string, string) => 'a = "verify";

/* Is there a way to catch undefined authorizationContext as it is with type-safety? */
let authorize : (Js.Dict.t(Js.Json.t), 'a, 'c) => Js.Promise.t(unit) = (event, context, callback) => {
  let buildPolicyDoc = (userId, effect, resource, context) => {
    let policy = {
      "principalId": userId,
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
    };
  };

  let verify = (token, methodArn) => {
    let decoded : {
      .
      "user" : {
        .
        "userId": string,
      },
    } = jwtVerify(token, Node.Process.process##env |> Js.Dict.unsafeGet(_, "jwt_secret"));

    /* skip user scope check now */
    let isAllowed = true;

    let effect = if (isAllowed) { "Allow" } else { "Deny" };
    let userId = decoded##user##userId;
    let authorizerContext = {
      "user": {
        "userId": userId,
      },
    };
    buildPolicyDoc(userId, effect, methodArn, authorizerContext);
  };

  let token = event
    |> Js.Dict.get(_, "authorizationToken")
    |> Js.Option.andThen((. v) => Js.Json.decodeString(v));
  let methodArn = event
    |> Js.Dict.get(_, "methodArn")
    |> Js.Option.andThen((. v) => Js.Json.decodeString(v));
  Js.log(event);
  Js.log(token |> Js.Option.isNone);

  let doc = verify(
    token |> Js.Option.getExn,
    methodArn |> Js.Option.getExn,
  );

  callback(Js.Json.null, doc);
  Js.Promise.resolve();
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

type t = Js.t({
  .
  userName: string,
  email: option(string),

  /* Currently, this should be 'Google' */
  idp: string,
  token: string,
});

[@bs.scope "JSON"] [@bs.val] external parse : string => Js.Json.t = "parse";
external decode : Js.Json.t => t = "%identity";
external encode : t => Js.Json.t = "%identity";

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

  let body = event
    |> bodyGet
    |> Js.Option.getExn
    |> parse
    |> decode;
  
  idpVerify(body##token)
  |> Js.Promise.then_(idpId => {
    DB.User.create(
      {
        "Google": idpId,
      },
      body##userName,
    )
    |> Js.Promise.then_(userId => {
      Js.Promise.all([|
        DB.User.createIdp("Google", idpId, userId),
        DB.User.createUsername(userId, body##userName),
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

  let body = event
    |> bodyGet
    |> Js.Option.getExn
    |> parse
    |> decode;
  
  Js.log(body);

  idpVerify(body##token)
  |> Js.Promise.then_(result => {
    Js.log(result);

    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(body |> encode),
      ()
    )
    |> Js.Promise.resolve;
  });
};
