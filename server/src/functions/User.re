type t = Js.t({
  .
  id: string,
  user_name: string,
  created_at: string,
  idp: Js.t({
    .
    google: string,
  })
});

external encode : t => Js.Json.t = "%identity";
let convert : DB.User.t => t = json => [%bs.obj {
  id: json##id,
  user_name: json##user_name,
  created_at: json##created_at,
  idp: {
    google: json##idp##google,
  },
}];

let getMe = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;
  open AwsLambda.APIGatewayProxy.EventRequestContext;
  let userId = event
    |> requestContextGet
    |> authorizerGet |> Js.Option.getExn
    |> Js.Dict.unsafeGet(_, "userId")
    |> Js.Json.decodeString |> Js.Option.getExn;

  DB.User.get(userId)
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.User.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.item |> convert |> encode),
      ()
    )
    |> Js.Promise.resolve;
  });
};
