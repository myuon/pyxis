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

let getMe = Controller.wrapper((event) => {
  let userId = event##requestContext##authorizer
    |> RawJson.decode
    |> x => x##userId;

  DB.User.get(userId)
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.User.t)) => {
    result.item
    |> convert
    |> encode
    |> Js.Promise.resolve;
  });
});
