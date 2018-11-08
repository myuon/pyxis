let get = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let ticketId = event
    |> pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.unsafeGet(_, "ticketId");

  DB.Page.list(
    ticketId
  )
  |> Js.Promise.then_((result : DB.QueryResult.many(Entity.Page.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(Js.Json.array(result.items |> Js.Array.map(Entity.Page.encode))),
      ()
    )
    |> Js.Promise.resolve;
  })
  |> Js.Promise.catch(err => {
    Result.make(
      ~statusCode=500,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringifyAny(err) |> Js.Option.getExn,
      ()
    )
    |> Js.Promise.resolve;
  });
};