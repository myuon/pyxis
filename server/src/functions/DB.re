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
[@bs.send] external delete : (client, Js.Json.t) => awsRequest = "delete";
[@bs.send] external batchWrite : (client, Js.Json.t) => awsRequest = "batchWrite";
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
    tickets: array(string),
    owned_by: string,
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("project-") |> x => x[1],
      title: t##title,
      tickets: t##tickets,
      owned_by: t##owned_by,
    }]
  };

  external encode : 't => Js.Json.t = "%identity";

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

  let create : Js.t({. owned_by: string, title: string}) => Js.Promise.t(Js.Json.t) = (json) => {
    let id = UUID.uuid();

    {
      "TableName": "entities",
      "Item": {
        "id": {j|project-$id|j},
        "sort": "detail",
        "title": json##title,
        "owned_by": json##owned_by,
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise;
  };

  let delete = (projectId) => {
    {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id",
      "ExpressionAttributeValues": {
        ":id": {j|project-$projectId|j},
      },
    }
    |> encode
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(x => x)
      |> Js.Promise.resolve;
    })
    |> Js.Promise.then_((result : QueryResult.many(Js.Json.t)) => {
      {
        "RequestItems": {
          "entities": result.items |> Belt.Array.map(_, item => {
            {
              "DeleteRequest": {
                "Key": {
                  "id": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "id"),
                  "sort": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "sort"),
                }
              }
            };
          })
        }
      }
      |> encode
      |> batchWrite(dbc,_)
      |> promise;
    });
  };
};

module Ticket = {
  type t = Js.t({
    .
    id: string,
    title: string,
    comment: int,
    assigned_to: array(string),
    belongs_to: Js.t({ .
      project: string,
    }),
    owned_by: string,
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
      },
      owned_by: t##owned_by,
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

  let create : Js.t({
    .
    title: string,
    assigned_to: array(string),
    belongs_to: Js.t({
      .
      project: string,
    }),
    owned_by: string,
  }) => Js.Promise.t(string) = (json) => {
    let id = UUID.uuid();

    {
      "TableName": "entities",
      "Item": {
        "id": {j|ticket-$id|j},
        "sort": "detail",
        "title": json##title,
        "comment": 0,
        "assigned_to": json##assigned_to,
        "belongs_to": {
          "project": json##belongs_to##project,
        },
        "owned_by": json##owned_by,
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise
    |> Js.Promise.then_(_ => {
      id
      |> Js.Promise.resolve;
    });
  };

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

  let update = (ticketId, label, value) => {
    {
      "TableName": "entities",
      "Key": {
        "id": {j|ticket-$ticketId|j},
        "sort": "detail",
      },
      "UpdateExpression": "set #label = :value",
      "ExpressionAttributeNames": { "#label": label },
      "ExpressionAttributeValues": { ":value": value },
    }
    |> encode
    |> update(dbc,_)
    |> promise;
  };

  let delete = (ticketId) => {
    {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id",
      "ExpressionAttributeValues": {
        ":id": {j|ticket-$ticketId|j},
      },
    }
    |> encode
    |> query(dbc,_)
    |> promise
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(x => x)
      |> Js.Promise.resolve;
    })
    |> Js.Promise.then_((result : QueryResult.many(Js.Json.t)) => {
      {
        "RequestItems": {
          "entities": result.items |> Belt.Array.map(_, item => {
            {
              "DeleteRequest": {
                "Key": {
                  "id": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "id"),
                  "sort": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "sort"),
                }
              }
            };
          })
        }
      }
      |> encode
      |> batchWrite(dbc,_)
      |> promise;
    });
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
    owned_by: string,
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
      },
      owned_by: t##owned_by,
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

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

  let create : Js.t({
    .
    id: string,
    title: string,
    content: string,
    belongs_to: Js.t({
      .
      project: string,
      ticket: string,
    }),
    owned_by: string,
  }) => Js.Promise.t(Js.Json.t) = (json) => {
    let ticketId = json##belongs_to##ticket;
    let pageId = json##id;

    {
      "TableName": "entities",
      "Item": {
        "id": {j|ticket-$ticketId|j},
        "sort": {j|page-$pageId|j},
        "title": json##title,
        "content": if (json##content != "") { json##content } else { [%raw {| null |}] },
        "belongs_to": {
          "project": json##belongs_to##project,
        },
        "owned_by": json##owned_by,
      }
    }
    |> encode
    |> put(dbc,_)
    |> promise;
  };
};

module Comment = {
  type t = Js.t({
    .
    id: string,
    sort: string,
    content: string,
    created_at: string,
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

  let create : (string, {
    .
    "content": string,
    "belongs_to": {
      .
      "project": string,
      "ticket": string,
    },
    "owned_by": string,
  }) => Js.Promise.t(Js.Json.t) = (commentId, item) => {
    let ticketId = item##belongs_to##ticket;

    {
      "TableName": "entities",
      "Item": [%bs.obj {
        id: {j|ticket-$ticketId|j},
        sort: {j|comment-$commentId|j},
        content: item##content,
        belongs_to: item##belongs_to,
        created_at: Js.Date.make() |> Js.Date.toISOString,
        owned_by: item##owned_by,
      }]
      |> (encode : t => Js.Json.t)
    }
    |> encode
    |> put(dbc,_)
    |> promise;
  };
};

module Idp = {
  type t = Js.t({
    .
    id: string,
    idp: string,
    owned_by: string,
  });

  external parse_ : Js.Json.t => 'a = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    /* Assume that idpId does not include hyphens: this is unsafe a bit */
    let [| _, idp, idpId |] = t##id |> Js.String.split("-");

    [%bs.obj {
      id: idpId,
      idp: idp,
      owned_by: t##owned_by,
    }]
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
  external parse_ : Js.Json.t => 'a = "%identity";
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
      |> QueryResult.parseOne(Idp.parse)
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
    |> Js.Promise.then_(_ => {
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
};
