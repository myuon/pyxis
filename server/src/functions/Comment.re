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
  |> Js.Promise.then_((result : DB.QueryResult.one(Entity.Ticket.t)) => {
    let ticketInfo : Entity.Ticket.t = {...result.item, comment: result.item.comment + 1};
    let comment : Entity.Comment.t = {...comment, id: ticketInfo.comment |> string_of_int};

    Js.Promise.all2(
      (
        DB.Comment.create(
          comment
        ),
        DB.Ticket.update(
          comment.belongs_to |> Js.Dict.get(_, "ticket") |> Belt.Option.getExn,
          "comment",
          ticketInfo.comment,
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

let list = (event, _context) => {
  open AwsLambda.APIGatewayProxy;

  let ticketId = event
    |> Event.pathParametersGet
    |> Js.Option.getExn
    |> Js.Dict.get(_, "ticketId")
    |> Js.Option.getExn;
  
  DB.Comment.list(ticketId)
  |> Js.Promise.then_((result : DB.QueryResult.many(Entity.Comment.t)) => {
    Result.make(
      ~statusCode=200,
      ~headers=Js.Dict.fromArray([| ("Access-Control-Allow-Origin", Js.Json.string("*")) |]),
      ~body=Js.Json.stringify(result.items |> Js.Array.map(Entity.Comment.encode) |> Js.Json.array),
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
