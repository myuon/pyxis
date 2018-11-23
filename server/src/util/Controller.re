type event = {
  .
  "pathParameters": Js.Dict.t(string),
};

type response = {
  .
  "statusCode": int,
  "headers": Js.Json.t,
  "body": string
};

exception Return(int, Js.Json.t);

let result : (~statusCode : int, ~body : Js.Json.t) => response = (~statusCode, ~body) => {
  {
    "statusCode": statusCode,
    "headers": {
      "Access-Control-Allow-Origin": "*",
    } |> RawJson.encode,
    "body": body |> Js.Json.stringify,
  }
};

let wrapper : ((event) => Js.Promise.t(Js.Json.t)) =>
              ((event, unit, unit) => Js.Promise.t(response))
= (func) => (event, _context, _callback) => {
  try (func(event)
    |> Js.Promise.then_(body => {
      result(
        ~statusCode=200,
        ~body=body,
      )
      |> Js.Promise.resolve;
    })
  ) {
    | Return(code, body) => result(
      ~statusCode=code,
      ~body=body,
    )
    |> Js.Promise.resolve;
  };
};
