let xray = fun (sdk) => if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  sdk
} else {
  [%bs.raw {| require('aws-xray-sdk-core').captureAWS(sdk) |}]
};

let aws = xray([%bs.raw {| require('aws-sdk') |}]);

type awsRequest;
type client;

[@bs.val] let dbc : client = if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  [%bs.raw {| new aws.DynamoDB.DocumentClient({
    region: 'localhost',
    endpoint: 'http://localhost:8000',
  }) |}]
} else {
  [%bs.raw {| new aws.DynamoDB.DocumentClient() |}]
};

[@bs.send] external get : (client, Js.Json.t) => awsRequest = "get";
[@bs.send] external query : (client, Js.Json.t) => awsRequest = "query";
[@bs.send] external update : (client, Js.Json.t) => awsRequest = "update";
[@bs.send] external promise : awsRequest => Js.Promise.t(Js.Json.t) = "promise";

module QueryResult = {
  type t('t) = {
    items: array('t),
    count: int,
    scannedCount: int,
  };

  let parse = (decoder, json) => {
    Json.Decode.{
      items: json |> field("Items", array(decoder)),
      count: json |> field("Count", int),
      scannedCount: json |> field("ScannedCount", int),
    }
  };
};

module Ticket = {
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
          ("Value", string(value)),
        ])),
      ])
    )
    |> update(dbc,_)
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
      |> QueryResult.parse(Entity.User.decode)
      |> Js.Promise.resolve(_)
    })
  };
};
