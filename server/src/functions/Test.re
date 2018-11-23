
let handler = Controller.wrapper((event) => {
  ("your value: " ++ (event##pathParameters |> Js.Dict.unsafeGet(_, "value")))
  |> Js.Json.string
  |> Js.Promise.resolve;
});
