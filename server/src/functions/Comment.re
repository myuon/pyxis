external decode : Js.Json.t => 'a = "%identity";

let create = (event, _context, _cb) => {
  open AwsLambda.APIGatewayProxy;

  let input : {
    .
    "content": string,
    "belongs_to": {
      .
      "project": string,
      "ticket": string,
    },
    "owned_by": string,
  } = event
    |> Event.bodyGet
    |> Js.Option.getExn
    |> Js.Json.parseExn
    |> decode;
  
  DB.Ticket.get(input##belongs_to##ticket)
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.Ticket.t)) => {
    let item = result.item;
    let commentIndex = item##comment + 1;
    
    /* This process is not safe -- do rollback if needed */
    Js.Promise.all2(
      (
        DB.Comment.create(
          commentIndex |> string_of_int,
          input
        ),
        DB.Ticket.update(
          input##belongs_to##ticket,
          "comment",
          commentIndex,
        )
      )
    )
    |> Js.Promise.then_(result => {
      Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Js.Json.stringifyAny(result) |> Js.Option.getExn,
        (),
      )
      |> Js.Promise.resolve;
    })
    |> Js.Promise.resolve;
  })
};

type t = Js.t({
  .
  id: string,
  content: string,
  created_at: string,
  owned_by: string,
  belongs_to: Js.t({
    .
    project: string,
    ticket: string
  }),
});

external parse_ : DB.Comment.t => t = "%identity";
let parse : DB.Comment.t => t = json => {
  [%bs.obj {
    id: json##id,
    content: json##content,
    created_at: json##created_at,
    owned_by: json##owned_by,
    belongs_to: json##belongs_to,
  }];
};
external encode : t => Js.Json.t = "%identity";

let list = (event, _context, _cb) => {
  open AwsLambda.APIGatewayProxy;

  let ticketId = event
    |> Event.pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.get(_, "ticketId")
    |> Js.Option.getExn;
  
  DB.Comment.list(ticketId)
  |> Js.Promise.then_((result : DB.QueryResult.many(DB.Comment.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.items
        |> Js.Array.map(item => item |> parse |> encode)
        |> Js.Json.array
      ),
      ()
    )
    |> Js.Promise.resolve;
  })
  |> Js.Promise.catch(err => {
    Result.make(
      ~statusCode=500,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringifyAny(err) |> Js.Option.getExn,
      ()
    )
    |> Js.Promise.resolve;
  });
};
