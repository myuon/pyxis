type t = Js.t({
  .
  id: string,
  title: string,
  content: string,
  belongs_to: Js.Dict.t(string),
});

let get = Controller.wrapper((event) => {
  let ticketId = event##pathParameters
    |> Js.Dict.unsafeGet(_, "ticketId");

  DB.Page.list(
    ticketId
  )
  |> Js.Promise.then_((result : DB.QueryResult.many(DB.Page.t)) => {
    result.items
    |> Js.Array.map(DB.Page.encode)
    |> Js.Json.array
    |> Js.Promise.resolve;
  });
});
