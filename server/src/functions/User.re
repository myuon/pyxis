let me = "1";

let getMe : AwsLambda.APIGatewayProxy.handler = (_event, _context, cb) => {
  open AwsLambda.APIGatewayProxy;

  DB.User.query(DB.User.me)
    |> Js.Promise.then_(result => {
      Js.log(result);

      cb(Js.null, Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Json.stringify(result),
        ()
      ));
      Js.Promise.resolve();
    })
};
