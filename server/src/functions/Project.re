type t = Js.t({
  .
  id: string,
  title: string,
  tickets: Js.Array.t(Js.t({
    .
    id: string,
    title: string,
    comment: int,
  })),
});

external encode : t => Js.Json.t = "%identity";
external decode : Js.Json.t => 'a = "%identity";

let listRecent = Controller.wrapper((event) => {
  let userId = event##requestContext
    |> Js.Dict.unsafeGet(_, "authorizer")
    |> Js.Json.parseExn
    |> RawJson.decode
    |> x => x##userId
  
  DB.Project.list(userId)
  |> Js.Promise.then_((result : DB.QueryResult.many(DB.Project.t)) => {
    let projects = result.items;

    DB.Ticket.list(userId)
    |> Js.Promise.then_((result : DB.QueryResult.many(DB.Ticket.t)) => {
      let tickets = result.items;

      let arr = {
        let projects : Belt.Map.String.t(t) = projects
          |> Js.Array.map(project => {
            [%bs.obj {
              id: project##id,
              title: project##title,
              tickets: [||],
            }]
          })
          |> Js.Array.map(value => (value##id, value))
          |> Belt.Map.String.fromArray;
        
        tickets
        |> Js.Array.forEach(ticket => {
          let _ = projects
          |> Belt.Map.String.getExn(_, ticket##belongs_to##project)
          |> x => x##tickets
          |> Js.Array.push([%bs.obj {
            id: ticket##id,
            title: ticket##title,
            comment: ticket##comment,
          }], _);
        }, _);

        projects
      };

      arr
      |> Belt.Map.String.valuesToArray
      |> Js.Array.map(encode)
      |> Js.Json.array
      |> Js.Promise.resolve;
    });
  })
});

let create = Controller.wrapper((event) => {
  let input : Js.t({
    .
    title: string,
    owned_by: string,
  }) = event##body
    |> Js.Json.parseExn
    |> decode;

  DB.Project.create(input);
});

let remove = Controller.wrapper((event) => {
  let projectId = event##pathParameters
    |> Js.Dict.unsafeGet(_, "projectId");

  Js.Promise.all2((
    DB.Project.delete(projectId),
    DB.Project.get(projectId)
    |> Js.Promise.then_((result : DB.QueryResult.one(DB.Project.t)) => {
      result.item##tickets
      |> Belt.Array.map(_, DB.Ticket.delete)
      |> Js.Promise.all
    })
  ))
  |> Js.Promise.then_(_ => {
    "OK"
    |> Js.Json.string
    |> Js.Promise.resolve;
  });
});
