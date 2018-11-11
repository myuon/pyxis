let me = "1";

type t = Js.t({
  .
  id: string,
  name: string,
  display_name: string,
  created_at: string,
});

external encode : t => Js.Json.t = "%identity";

let getMe = (_event, _context) => {
  open AwsLambda.APIGatewayProxy;

  let user = [%bs.obj {
    id: "1",
    name: "myuon",
    display_name: "myuon",
    created_at: Js.Date.make() |> Js.Date.toISOString,
  }];

  Result.make(
    ~statusCode=200,
    ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
    ~body=Js.Json.stringify(user |> encode),
    ()
  )
  |> Js.Promise.resolve;
};
