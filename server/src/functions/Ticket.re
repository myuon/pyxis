let get = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let ticketId = event
    |> pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.get(_, "ticketId")
    |> Js.Option.getExn;

  DB.Ticket.get(ticketId)
  |> Js.Promise.then_((result : DB.QueryResult.one(Entity.Ticket.t)) => {
    let result = Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.item |> Entity.Ticket.encode),
      ()
    );

    Js.Promise.resolve(result);
  })
  |> Js.Promise.catch(err => {
    let result = Result.make(
      ~statusCode=500,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringifyAny(err) |> Js.Option.getExn,
      ()
    );

    Js.Promise.resolve(result);
  })
};
