let me = "1";

let listRecent = (_event, _context) => {
  open AwsLambda.APIGatewayProxy;

  DB.Project.list(me)
  |> Js.Promise.then_((result : DB.QueryResult.many(Entity.Project.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(Js.Json.array(result.items |> Js.Array.map(Entity.Project.encode))),
      ()
    )
    |> Js.Promise.resolve
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
