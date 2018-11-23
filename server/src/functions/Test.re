
let handler = Controller.wrapper((event) => {
  let value = event##pathParameters |> Js.Dict.unsafeGet(_, "value");

  if (value == "error") {
    raise(Controller.Return(400, "error happened!" |> Js.Json.string));
  };

  ("your value: " ++ value)
  |> Js.Json.string
  |> Js.Promise.resolve;
});
