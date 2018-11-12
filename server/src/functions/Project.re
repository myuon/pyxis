let me = "1";

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

let listRecent = (_event, _context) => {
  open AwsLambda.APIGatewayProxy;

  DB.Project.list(me)
  |> Js.Promise.then_((result : DB.QueryResult.many(DB.Project.t)) => {
    let projects = result.items;

    DB.Ticket.list(me)
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

      Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Js.Json.stringify(Js.Json.array(arr |> Belt.Map.String.valuesToArray |> Js.Array.map(encode))),
        ()
      )
      |> Js.Promise.resolve;
    });
  })
};

let get = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let projectId = event
    |> pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.unsafeGet(_, "projectId");

  DB.Project.get(me, projectId)
  |> Js.Promise.then_((result : DB.QueryResult.one(Entity.Project.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.item |> Entity.Project.encode),
      ()
    )
    |> Js.Promise.resolve
  })
};