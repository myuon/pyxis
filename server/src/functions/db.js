'use strict';

const lib = require('pyxis-lib');

const xray = (sdk) => process.env.IS_OFFLINE ? sdk : require('aws-xray-sdk-core').captureAWS(sdk);
const AWS = xray(require('aws-sdk'));
const db = process.env.IS_OFFLINE
  ? new AWS.DynamoDB.DocumentClient({
    region: 'localhost',
    endpoint: 'http://localhost:8000'
  })
  : new AWS.DynamoDB.DocumentClient();

const wrap = async (req) => {
  return await req.promise().catch(err => {
    console.error(`[PyxisError]: ${err}`);
    throw new Error(JSON.stringify(err));
  });
};

const convert = {
  ticket: {
    fromJSON: (json) => {
      return lib.validate(lib.ticket, {
        id: json.id.split('-')[1],
        title: json.title,
        assigned_to: json.assigned_to,
        comment: json.comment,
      });
    },
    toJSON: (ticket) => {
      return {
        id: ticket.belongs_to,
        sort: 'detail',
        title: ticket.title,
        assigned_to: ticket.assigned_to,
        comment: ticket.comment,
      };
    },
    asAttribute: (label, value) => {
      return {
        [label]: {
          Action: 'PUT',
          Value: value,
        }
      };
    },
  },
  page: {
    fromJSON: (json) => {
      return lib.validate(lib.page, {
        id: json.sort.split('-')[1],
        title: json.title,
        content: json.content,
        belongs_to: json.id,
      });
    },
    toJSON: (page) => {
      return {
        id: ticket.belongs_to,
        sort: `page-${page.id}`,
        title: page.title,
        content: page.content,
      };
    },
    asAttribute: (label, value) => {
      return {
        [label]: {
          Action: 'PUT',
          Value: value,
        }
      };
    },
  },
  comment: {
    fromJSON: (json) => {
      return lib.validate(lib.comment, {
        id: json.sort.split('-')[1],
        content: json.content,
        created_at: json.created_at,
        owner: json.owner,
        belongs_to: json.id,
      });
    },
    toJSON: (comment) => {
      return {
        id: comment.belongs_to,
        sort: `comment-${comment.id}`,
        content: comment.content,
        owner: comment.owner,
        created_at: comment.created_at,
      }
    },
    asAttribute: (label, value) => {
      return {
        [label]: {
          Action: 'PUT',
          Value: value,
        }
      };
    },
  },
};

const tables = {
  ticket: {
    get: async (projectId, ticketId) => {
      const res = await wrap(db.get({
        TableName: 'tickets',
        Key: {
          id: `${projectId}-${ticketId}`,
          sort: 'detail'
        }
      }));

      return convert.ticket.fromJSON(res['Item']);
    },
    list: async (projectId, ticketId) => {
      const res = await wrap(db.query({
        TableName: 'tickets',
        KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
        ExpressionAttributeValues: {
          ':id': `${projectId}-${ticketId}`,
          ':sort': `comment`,
        }
      }));

      return res['Items'].map(item => convert.ticket.fromJSON(item));
    },
    update: async (projectId, ticketId, label, value) => {
      const res = await wrap(db.update({
        TableName: 'tickets',
        Key: {
          id: `${projectId}-${ticketId}`,
          sort: 'detail',
        },
        AttributeUpdates: convert.ticket.asAttribute(label, value),
      }));

      return res;
    },
    create: async (projectId, ticketId, item) => {
      const res = await wrap(db.put({
        TableName: 'tickets',
        Item: convert.ticket.toJSON(item),
      }));

      return res;
    },
  },
  page: {
    list: async (projectId, ticketId) => {
      const res = await wrap(db.query({
        TableName: 'tickets',
        KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
        ExpressionAttributeValues: {
          ':id': `${projectId}-${ticketId}`,
          ':sort': 'page',
        }
      }));

      return res['Items'].map(item => convert.page.fromJSON(item));
    },
  },
  comment: {
    list: async (projectId, ticketId) => {
      const res = await wrap(db.query({
        TableName: 'tickets',
        KeyConditionExpression: 'id = :id and begins_with(sort, :sort)',
        ExpressionAttributeValues: {
          ':id': `${projectId}-${ticketId}`,
          ':sort': 'comment',
        }
      }));

      return res['Items'].map(item => convert.comment.fromJSON(item));
    },
    create: async (projectId, ticketId, item) => {
      const res = await wrap(db.put({
        TableName: 'tickets',
        Item: convert.comment.toJSON(item),
      }));

      return res;
    },
  },
}

module.exports = {
  tables,
};
