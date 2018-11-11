let create = (event, _context) => {
  open AwsLambda.APIGatewayProxy;

  let comment = event
    |> Event.bodyGet
    |> Js.Option.getExn
    |> Js.Json.parseExn
    |> Entity.Comment.decode;
  
  DB.Ticket.get(
    comment.belongs_to |> Js.Dict.get(_, "ticket") |> Js.Option.getExn
  )
  |> Js.Promise.then_((result : DB.QueryResult.one(DB.Ticket.t)) => {
    let item = result.item;
    let ticketInfo = [%bs.obj {
      id: item##id,
      title: item##title,
      comment: item##comment + 1,
      belongs_to: {
        project: item##belongs_to##project
      }
    }];
    let comment : Entity.Comment.t = {...comment, id: ticketInfo##comment |> string_of_int};

    Js.Promise.all2(
      (
        DB.Comment.create(
          comment
        ),
        DB.Ticket.update(
          comment.belongs_to |> Js.Dict.get(_, "ticket") |> Belt.Option.getExn,
          "comment",
          ticketInfo##comment,
        )
      )
    )
    |> Js.Promise.then_(_ => {
      Result.make(
        ~statusCode=200,
        ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
        ~body=Json.stringify(Js.Json.null),
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
  created_at: Js.Date.t,
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

let list = (event, _context) => {
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
