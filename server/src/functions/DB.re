let xray = fun (sdk) => if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  sdk
} else {
  [%bs.raw {| require('aws-xray-sdk-core').captureAWS(sdk) |}]
};

let aws = xray([%bs.raw {| require('aws-sdk') |}]);

type awsRequest;
type client;

let dbc : client = if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  [%bs.raw {| new aws.DynamoDB.DocumentClient({
    region: 'localhost',
    endpoint: 'http://localhost:8000',
  }) |}]
} else {
  [%bs.raw {| new aws.DynamoDB.DocumentClient() |}]
};

[@bs.send] external get : (client, Js.Json.t) => awsRequest = "get";
[@bs.send] external put : (client, Js.Json.t) => awsRequest = "put";
[@bs.send] external query : (client, Js.Json.t) => awsRequest = "query";
[@bs.send] external update : (client, Js.Json.t) => awsRequest = "update";
[@bs.send] external promise : awsRequest => Js.Promise.t(Js.Json.t) = "promise";

module QueryResult = {
  type one('t) = {
    item: 't,
  };

  type many('t) = {
    items: array('t),
    count: int,
    scannedCount: int,
  };

  let parseOne = (decoder, json) => {
    Json.Decode.{
      item: json |> field("Item", decoder),
    }
  };

  let parseMany = (decoder, json) => {
    Json.Decode.{
      items: json |> field("Items", array(decoder)),
      count: json |> field("Count", int),
      scannedCount: json |> field("ScannedCount", int),
    }
  };
};

module Project = {
  let decodeDBObject : Js.Json.t => Entity.Project.t = json => {
    Json.Decode.{
      id: json |> field("sort", string) |> Js.String.split("project-") |> x => x[1],
      title: json |> field("title", string),
      tickets: json |> field("tickets", array(string)),
    }
  };

  let list = (userId) => {
    Json.Encode.(
      object_([
        ("TableName", string("users")),
        ("KeyConditionExpression", string("id = :id and begins_with(sort, :sort)")),
        ("ExpressionAttributeValues", object_([
          (":id", string(userId)),
          (":sort", string("project")),
        ]))
      ])
    )
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(decodeDBObject)
      |> Js.Promise.resolve
    })
  };

  let get = (userId, projectId) => {
    Json.Encode.(
      object_([
        ("TableName", string("users")),
        ("Key", object_([
          ("id", string(userId)),
          ("sort", string({j|project-$projectId|j})),
        ]))
      ])
    )
    |> get(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(decodeDBObject)
      |> Js.Promise.resolve
    })
  };
};

module Ticket = {
  let decodeDBObject : Js.Json.t => Entity.Ticket.t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      title: json |> field("title", string),
      assigned_to: json |> field("assigned_to", array(string)),
      comment: json |> field("comment", int),
      belongs_to: json |> field("belongs_to", dict(string)),
    }
  };

  let get = (ticketId) => {
    Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("Key", object_([
          ("id", string(ticketId)),
          ("sort", string("detail"))
        ]))
      ])
    )
    |> get(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(decodeDBObject)
      |> Js.Promise.resolve
    })
  };

  let update = (ticketId, label, value) => {
    Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("Key", object_([
          ("id", string(ticketId)),
          ("sort", string("detail")),
        ])),
        ("AttributeUpdates", object_([
          ("Action", string("PUT")),
          ("Value", int(value)),
        ])),
      ])
    )
    |> update(dbc,_)
    |> promise
  };
};

module Page = {
  let decodeDBObject : Js.Json.t => Entity.Page.t = json => {
    Json.Decode.{
      id: json |> field("sort", string) |> Js.String.split("-",_) |> x => x[1],
      title: json |> field("title", string),
      content: json |> field("content", string),
      belongs_to: Js.Dict.fromArray([|
        ("project", json |> field("belongs_to", dict(string)) |> Js.Dict.get(_, "project") |> Belt.Option.getExn),
        ("ticket", json |> field("id", string)),
      |])
    }
  };

  let list = (ticketId) => {
    Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("KeyConditionExpression", string("id = :id and begins_with(sort, :sort)")),
        ("ExpressionAttributeValues", object_([
          (":id", string(ticketId)),
          (":sort", string("page")),
        ]))
      ])
    )
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(decodeDBObject)
      |> Js.Promise.resolve
    })
  };
};

module Comment = {
  let decodeDBObject : Js.Json.t => Entity.Comment.t = json => {
    Json.Decode.{
      id: json |> field("sort", string) |> Js.String.split("-",_) |> x => x[1],
      content: json |> field("content", string),
      created_at: json |> field("created_at", date),
      owner: json |> field("owner", string),
      belongs_to: Js.Dict.fromArray([|
        ("project", json |> field("belongs_to", dict(string)) |> Js.Dict.get(_, "project") |> Belt.Option.getExn),
        ("ticket", json |> field("id", string)),
      |])
    }
  };

  let encodeDBObject : Entity.Comment.t => Js.Json.t = comment => {
    Json.Encode.(
      object_([
        ("id", string(comment.belongs_to |> Js.Dict.unsafeGet(_, "ticket"))),
        ("sort", string({j|comment-$comment.id|j})),
        ("content", string(comment.content)),
        ("owner", string(comment.owner)),
        ("created_at", string(comment.created_at |> Js.Date.toISOString)),
        ("belongs_to", object_([
          ("project", string(comment.belongs_to |> Js.Dict.unsafeGet(_, "project"))),
        ]))
      ])
    )
  };

  let list = (ticketId) => {
    Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("KeyConditionExpression", string("id = :id and begins_with(sort, :sort)")),
        ("ExpressionAttributeValues", object_([
          (":id", string(ticketId)),
          (":sort", string("comment")),
        ]))
      ])
    )
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(decodeDBObject)
      |> Js.Promise.resolve;
    })
  };

  let create = (item) => {
    Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("Item", item |> encodeDBObject),
      ])
    )
    |> put(dbc,_)
    |> promise
  };
};

module User = {
  let me = "1";

  let query = (userId) => {
    Json.Encode.(
      object_([
        ("TableName", string("users")),
        ("KeyConditionExpression", string("id = :id and sort = :sort")),
        ("ExpressionAttributeValues", object_([
          (":id", string(userId)),
          (":sort", string("detail")),
        ])),
      ])
    )
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result 
      |> QueryResult.parseMany(Entity.User.decode)
      |> Js.Promise.resolve
    })
  };
};
