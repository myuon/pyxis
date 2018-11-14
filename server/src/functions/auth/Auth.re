type t = Js.t({
  .
  userId: string,
  email: option(string),
  idp: string,
});

[@bs.scope "JSON"] [@bs.val] external parse : string => Js.Json.t = "parse";
external decode : Js.Json.t => t = "%identity";
external encode : t => Js.Json.t = "%identity";

let signUp = (event, _context) => {
  open AwsLambda.APIGatewayProxy;
  open AwsLambda.APIGatewayProxy.Event;

  let body = event
    |> bodyGet
    |> Js.Option.getExn
    |> parse
    |> decode;
  
  Js.log(body);

  Result.make(
    ~statusCode=200,
    ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
    ~body=Js.Json.stringify(body |> encode),
    ()
  )
  |> Js.Promise.resolve;
};
