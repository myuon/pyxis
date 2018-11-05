let xray = fun (sdk) => if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  sdk
} else {
  [%bs.raw {| require('aws-xray-sdk-core').captureAWS(sdk) |}]
};

let aws = xray([%bs.raw {| require('aws-sdk') |}]);

let dbc : {
  .
  get : Js.Json.t => { . promise : unit => Js.Promise.t(Js.Json.t) },
  query : Js.Json.t => { . promise : unit => Js.Promise.t(Js.Json.t) },
  update : Js.Json.t => { . promise : unit => Js.Promise.t(Js.Json.t) },
} = if (Node.Process.process##env |> Js.Dict.get(_, "IS_OFFLINE") == Some("true")) {
  [%bs.raw {| new aws.DynamoDB.DocumentClient({
    region: 'localhost',
    endpoint: 'http://localhost:8000',
  }) |}]
} else {
  [%bs.raw {| new aws.DynamoDB.DocumentClient() |}]
};

module Ticket = {
  let get = (ticketId) => {
    dbc#get(Json.Encode.(
      object_([
        ("TableName", string("tickets")),
        ("Key", object_([
          ("id", string(ticketId)),
          ("sort", string("detail"))
        ]))
      ])
    ))#promise()
  };

  let update = (ticketId, label, value) => {
    dbc#update(Json.Encode.(
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
    ))#promise()
  };
};

module User = {
  let me = "1";

  let query = (userId) => {
    dbc#query(Json.Encode.(
      object_([
        ("TableName", string("users")),
        ("KeyConditionExpression", string("id = :id and sort = :sort")),
        ("ExpressionAttributeValues", object_([
          (":id", string(userId)),
          (":sort", string("detail")),
        ])),
      ])
    ))#promise()
  };
};
