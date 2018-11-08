let me = "1";

let getMe = (_event, _context) => {
  open AwsLambda.APIGatewayProxy;

  DB.User.query(DB.User.me)
    |> Js.Promise.then_((result : DB.QueryResult.many(Entity.User.t)) => {
      Js.log(result.items[0] |> Entity.User.encode);

      let result = Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Js.Json.stringify(result.items[0] |> Entity.User.encode),
        ()
      );

      Js.Promise.resolve(result);
    })
    |> Js.Promise.catch(err => {
      Js.log(err);

      let result = Result.make(
        ~statusCode=500,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body="Error!",
        ()
      );

      Js.Promise.resolve(result);
    })
};
