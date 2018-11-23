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

let get = Controller.wrapper((event) => {
  let ticketId = event##pathParameters
    |> Js.Dict.unsafeGet(_, "ticketId");

  DB.Ticket.get(ticketId)
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.Ticket.t)) => {
    result.item
    |> RawJson.encode
    |> Js.Promise.resolve;
  });
});

let create = Controller.wrapper((event) => {
  let input : Js.t({
    .
    title: string,
    assigned_to: array(string),
    belongs_to: Js.t({
      .
      project: string,
    }),
    owned_by: string,
  }) = event##body
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
      result
      |> RawJson.encode
      |> Js.Promise.resolve;
    });
  });
});

let remove = Controller.wrapper((event) => {
  let ticketId = event##pathParameters
    |> Js.Dict.unsafeGet(_, "ticketId");

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
    result
    |> RawJson.encode
    |> Js.Promise.resolve;
  });
});
