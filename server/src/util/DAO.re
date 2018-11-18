
/* DynamoDB DocumentClient wrapper module */
module DBC = {
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
};

external encode : 'a => Js.Json.t = "%identity";

let create : Js.t('a) => Js.Promise.t(Js.Json.t) = (item) => {
  {
    "TableName": "entities",
    "Item": item |> encode,
  }
  |> encode
  |> DBC.put(DBC.dbc,_)
  |> DBC.promise;
};

let listByOwner : (~owner: string, ~idPrefix: string, ~sort: string = ?, unit) => Js.Promise.t(Js.Json.t) = (~owner, ~idPrefix, ~sort=?, ()) => {
  switch sort {
    | None => {
      "TableName": "entities",
      "IndexName": "owners",
      "KeyConditionExpression": "owned_by = :owned_by and begins_with(id, :id)",
      "ExpressionAttributeValues": {
        ":owned_by": owner,
        ":id": idPrefix,
      }
    } |> encode
    | Some(sort) => {
      "TableName": "entities",
      "IndexName": "owners",
      "KeyConditionExpression": "owned_by = :owned_by and begins_with(id, :id)",
      "FilterExpression": "sort = :sort",
      "ExpressionAttributeValues": {
        ":owned_by": owner,
        ":id": idPrefix,
        ":sort": sort
      }
    } |> encode
  }
  |> DBC.query(DBC.dbc,_)
  |> DBC.promise;
};

let list : (~id: string, ~sortPrefix: string = ?, unit) => Js.Promise.t(Js.Json.t) = (~id, ~sortPrefix=?, ()) => {
  switch sortPrefix {
    | None => {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id",
      "ExpressionAttributeValues": {
        ":id": id,
      }
    } |> encode
    | Some(sortPrefix) => {
      "TableName": "entities",
      "KeyConditionExpression": "id = :id and begins_with(sort, :sort)",
      "ExpressionAttributeValues": {
        ":id": id,
        ":sort": sortPrefix,
      }
    } |> encode
  }
  |> encode
  |> DBC.query(DBC.dbc,_)
  |> DBC.promise
};

let get : (~id: string, ~sort: string) => Js.Promise.t(Js.Json.t) = (~id, ~sort) => {
  {
    "TableName": "entities",
    "Key": {
      "id": id,
      "sort": sort,
    },
  }
  |> encode
  |> DBC.get(DBC.dbc,_)
  |> DBC.promise;
};

let update : (~id: string, ~sort: string, ~label: string, ~value: 'a) => Js.Promise.t(Js.Json.t) = (~id, ~sort, ~label, ~value) => {
  {
    "TableName": "entities",
    "Key": {
      "id": id,
      "sort": sort,
    },
    "UpdateExpression": "set #label = :value",
    "ExpressionAttributeNames": { "#label": label },
    "ExpressionAttributeValues": { ":value": value },
  }
  |> encode
  |> DBC.update(DBC.dbc,_)
  |> DBC.promise;
};

let batchDelete : array({. "id": string, "sort": string}) => Js.Promise.t(Js.Json.t) = (items) => {
  {
    "RequestItems": {
      "entities": items |> Belt.Array.map(_, item => {
        {
          "DeleteRequest": {
            "Key": {
              "id": item##id,
              "sort": item##sort,
            }
          }
        };
      })
    }
  }
  |> encode
  |> DBC.batchWrite(DBC.dbc,_)
  |> DBC.promise;
};
