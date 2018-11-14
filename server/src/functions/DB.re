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
  type t = Js.t({
    .
    id: string,
    title: string,
    owned_by: string,
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("project-") |> x => x[1],
      title: t##title,
      owned_by: t##owned_by,
    }]
  };

  external encode : t => Js.Json.t = "%identity";

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
        ("TableName", string("entities")),
        ("IndexName", string("owners")),
        ("KeyConditionExpression", string("owned_by = :owner and begins_with(id, :id)")),
        ("ExpressionAttributeValues", object_([
          (":owner", string(userId)),
          (":id", string("project")),
        ]))
      ])
    )
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
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

  type t = Js.t({
    .
    id: string,
    title: string,
    comment: int,
    assigned_to: array(string),
    belongs_to: Js.t({ .
      project: string,
    }),
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("ticket-") |> x => x[1],
      title: t##title,
      comment: t##comment,
      assigned_to: t##assigned_to,
      belongs_to: {
        project: t##belongs_to##project,
      }
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

  let list = (userId) => {
    {
      "TableName": "entities",
      "IndexName": "owners",
      "KeyConditionExpression": "owned_by = :owned_by and begins_with(id, :id)",
      "FilterExpression": "sort = :sort",
      "ExpressionAttributeValues": {
        ":owned_by": userId,
        ":id": "ticket",
        ":sort": "detail",
      }
    }
    |> encode
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve;
    })
  };

  let get = (ticketId) => {
    {
      "TableName": "entities",
      "Key": {
        "id": {j|ticket-$ticketId|j},
        "sort": "detail",
      }
    }
    |> encode
    |> get(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let update = (ticketId, _label, value) => {
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
  type t = Js.t({
    .
    id: string,
    title: string,
    content: string,
    belongs_to: Js.t({
      .
      project: string,
      ticket: string,
    }),
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("ticket-") |> x => x[1],
      title: t##title,
      content: t##content,
      belongs_to: {
        project: t##belongs_to##project,
        ticket: t##belongs_to##ticket,
      }
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

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
    {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id and begins_with(sort, :sort)",
      "ExpressionAttributeValues": {
        ":id": {j|ticket-$ticketId|j},
        ":sort": "page",
      }
    }
    |> encode
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve
    })
  };
};

module Comment = {
  type t = Js.t({
    .
    id: string,
    sort: string,
    content: string,
    created_at: Js.Date.t,
    owned_by: string,
    belongs_to: Js.t({
      .
      ticket: string,
      project: string,
    }),
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##sort |> Js.String.split("comment-", _) |> x => x[1],
      sort: t##sort,
      content: t##content,
      created_at: t##created_at,
      owned_by: t##owned_by,
      belongs_to: {
        ticket: t##belongs_to##ticket,
        project: t##belongs_to##project,
      },
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

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
    {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id and begins_with(sort, :sort)",
      "ExpressionAttributeValues": {
        ":id": {j|ticket-$ticketId|j},
        ":sort": "comment",
      },
    }
    |> encode
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
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
  type t = Js.t({
    .
    id: string,
    sort: string,
    idp: Js.t({
      .
      google: string,
    }),
    created_at: Js.Date.t,
    user_name: string,
  });

  external encode : 'a => Js.Json.t = "%identity";
  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("user-", _) |> x => x[1],
      sort: t##sort,
      idp: {
        google: t##idp##google,
      },
      created_at: t##created_at,
      user_name: t##user_name,
    }]
  };

  let findById = userId => {
    {
      "TableName": "entities",
      "Key": {
        "id": {j|user-$userId|j},
        "sort": "detail",
      }
    }
    |> encode
    |> get(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let findViaIdp = (idp, idp_id) => {
    {
      "TableName": "entities",
      "Key": {
        "id": {j|idp-$idp-$idp_id|j},
        "sort": "detail",
      },
    }
    |> encode
    |> get(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let createIdp = (idp, idpId, userId) => {
    {
      "TableName": "entities",
      "Item": {
        "id": {j|idp-$idp-$idpId|j},
        "sort": "detail",
        "owned_by": userId
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise;
  };

  let create = (idp, userName) => {
    let userId = UUID.uuid();

    {
      "TableName": "entities",
      "Item": {
        "id": {j|user-$userId|j},
        "sort": "detail",
        "idp": idp,
        "created_at": Js.Date.make() |> Js.Date.toISOString,
        "user_name": userName,
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      userId
      |> Js.Promise.resolve;
    });
  };

  let createUsername = (userId, userName) => {
    {
      "TableName": "users",
      "Item": {
        "user_name": userName,
        "id": userId,
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise;
  };

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
