let endpoint = "http://localhost:3000";

[@bs.scope "JSON"] [@bs.val] external parse : string => 'a = "parse";

let fetch = (api) => {
  Fetch.fetch({j|$endpoint$api|j})
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> parse |> Js.Promise.resolve);
};

let client : {
  .
  "user": {
    .
    "me": unit => Js.Promise.t({
      .
      "id": string
    }),
  },
  "ticket": {
    .
    "get": unit => Js.Promise.t({
      .
      "id": string,
      "title": string,
      "comment": int,
      "assigned_to": array(string),
      "belongs_to": {
        .
        "project": string,
      },
    })
  },
  "comment": {
    .
    "list": unit => Js.Promise.t(array({
      .
      "id": string,
      "content": string,
      "created_at": Js.Date.t,
      "owned_by": string,
      "belongs_to": {
        .
        "project": string,
      },
    }))
  },
  "page": {
    .
    "list": string => Js.Promise.t(array({
      .
      "id": string,
      "title": string,
      "content": string,
      "belongs_to": {
        .
        "ticket": string,
        "project": string,
      },
    }))
  },
  "project": {
    .
    "listRecent": unit => Js.Promise.t(array({
      .
      "id": string,
      "title": string,
      "owned_by": string,
    })),
  },
} = {
  "user": {
    "me": () => fetch("/users/me"),
  },
  "ticket": {
    "get": (ticketId) => {
      let ticket = fetch({j|/tickets/$ticketId|j});
      ticket;
    },
  },
  "comment": {
    "list": (ticketId) => fetch({j|/tickets/$ticketId/comments|j}),
  },
  "page": {
    "list": (ticketId) => fetch({j|/tickets/$ticketId/pages|j}),
  },
  "project": {
    "listRecent": () => fetch("/projects/recent"),
  },
};
