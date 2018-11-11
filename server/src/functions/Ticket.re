type t = Js.t({
  .
  id: string,
  title: string,
  comment: int,
  assigned_to: array(string),
  belongs_to: Js.t({
    .
    project: string,
  }),
});

external parse : DB.Ticket.t => t = "%identity";
external encode : t => Js.Json.t = "%identity";

let get = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let ticketId = event
    |> pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.get(_, "ticketId")
    |> Js.Option.getExn;

  DB.Ticket.get(ticketId)
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.Ticket.t)) => {
    let result = Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.item |> parse |> encode),
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
