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
external decode : Js.Json.t => 'a = "%identity";
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

let create = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let input : Js.t({
    .
    title: string,
    assigned_to: array(string),
    belongs_to: Js.t({
      .
      project: string,
    }),
    owned_by: string,
  }) = event
    |> bodyGet
    |> Js.Option.getExn
    |> Js.Json.parseExn
    |> decode;

  DB.Ticket.create(input)
  |> Js.Promise.then_((ticketId : string) => {
    Js.Promise.all2((
      DB.Project.get(input##belongs_to##project)
      |> Js.Promise.then_((result: DB.QueryResult.one(DB.Project.t)) => {
        let tickets = result.item##tickets;
        let _ = tickets |> Js.Array.push(ticketId);

        DB.Project.update(
          input##belongs_to##project,
          "tickets",
          tickets
        );
      }),
      DB.Page.create([%bs.obj {
        id: "1",
        title: "New Document",
        content: "",
        belongs_to: {
          project: input##belongs_to##project,
          ticket: ticketId,
        },
        owned_by: input##owned_by,
      }])
    ))
    |> Js.Promise.then_(result => {
      Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Js.Json.stringifyAny(result) |> Js.Option.getExn,
        ()
      )
      |> Js.Promise.resolve;
    })
    |> Js.Promise.resolve;
  });
};

let remove = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let ticketId = event
    |> pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.get(_, "ticketId")
    |> Js.Option.getExn;

  Js.Promise.all2((
    DB.Ticket.delete(ticketId),
    DB.Ticket.get(ticketId)
    |> Js.Promise.then_((result : DB.QueryResult.one(DB.Ticket.t)) => {
      let projectId = result.item##belongs_to##project;
      DB.Project.get(projectId)
      |> Js.Promise.then_((result : DB.QueryResult.one(DB.Project.t)) => {
        let tickets = result.item##tickets;

        DB.Project.update(
          projectId,
          "tickets",
          tickets |> Js.Array.filter(item => item != ticketId),
        );
      });
    })
  ))
  |> Js.Promise.then_(result => {
      Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Js.Json.stringifyAny(result) |> Js.Option.getExn,
        ()
      )
      |> Js.Promise.resolve;
  });
};
