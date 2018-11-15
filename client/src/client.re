let endpoint = "http://localhost:3000";

[@bs.scope "JSON"] [@bs.val] external parse : string => 'a = "parse";

let fetch = (api) => {
  Fetch.fetchWithInit(
    {j|$endpoint$api|j},
    Fetch.RequestInit.make(
      ~method_=Get,
      ~credentials=Include,
      ()
    )
  )
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> parse |> Js.Promise.resolve);
};

let post = (api, body) => {
  Fetch.fetchWithInit(
    {j|$endpoint$api|j},
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(body)),
      ~credentials=Include,
      ()
    )
  )
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> Js.Json.parseExn |> Js.Promise.resolve);
};

let client : {
  .
  "auth": {
    .
    "signIn": Js.Json.t => Js.Promise.t(Js.Json.t),
  },
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
  "auth": {
    "signIn": (json) => post("/auth/signIn", json)
  },
  "user": {
    "me": () => fetch("/users/me"),
  },
  "ticket": {
    "get": (ticketId) => fetch({j|/tickets/$ticketId|j}),
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
