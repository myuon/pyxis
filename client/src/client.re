let endpoint = "http://localhost:3000";
external decode : Js.Json.t => 'a = "%identity";
external encode : 'a => Js.Json.t = "%identity";

let get = (api) => {
  open JustgageReasonCookie;
  let token = Cookie.getAsString("token");

  Fetch.fetchWithInit(
    {j|$endpoint$api|j},
    Fetch.RequestInit.make(
      ~method_=Get,
      ~headers=Fetch.HeadersInit.make({"Authorization": {j|Bearer $token|j}}),
      ~credentials=Include,
      ()
    )
  )
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> Js.Json.parseExn |> decode |> Js.Promise.resolve);
};

let delete = (api) => {
  open JustgageReasonCookie;
  let token = Cookie.getAsString("token");

  Fetch.fetchWithInit(
    {j|$endpoint$api|j},
    Fetch.RequestInit.make(
      ~method_=Delete,
      ~headers=Fetch.HeadersInit.make({"Authorization": {j|Bearer $token|j}}),
      ~credentials=Include,
      ()
    )
  )
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> Js.Json.parseExn |> decode |> Js.Promise.resolve);
};

let post = (api, body) => {
  open JustgageReasonCookie;
  let token = Cookie.getAsString("token");

  Fetch.fetchWithInit(
    {j|$endpoint$api|j},
    Fetch.RequestInit.make(
      ~method_=Post,
      ~body=Fetch.BodyInit.make(Js.Json.stringify(body)),
      ~headers=Fetch.HeadersInit.make({"Authorization": {j|Bearer $token|j}}),
      ~credentials=Include,
      ()
    )
  )
  |> Js.Promise.then_(Fetch.Response.text)
  |> Js.Promise.then_(result => result |> Js.Json.parseExn |> decode |> Js.Promise.resolve);
};

let client : {
  .
  "auth": {
    .
    "signIn": {. "token": string} => Js.Promise.t({. "token": string }),
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
    }),
    "create": {
      .
      "title": string,
      "assigned_to": array(string),
      "belongs_to": {
        .
        "project": string,
      },
      "owned_by": string,
    } => Js.Promise.t(Js.Json.t),
    "delete": string => Js.Promise.t(Js.Json.t),
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
    "create": {. "title": string, "owned_by": string} => Js.Promise.t(Js.Json.t),
    "listRecent": unit => Js.Promise.t(array({
      .
      "id": string,
      "title": string,
      "owned_by": string,
    })),
  },
} = {
  "auth": {
    "signIn": (json) => {
      open JustgageReasonCookie;

      post("/auth/signIn", json |> encode)
      |> Js.Promise.then_(result => {
        Cookie.setString("token", result##token);

        result
        |> Js.Promise.resolve;
      })
    }
  },
  "user": {
    "me": () => get("/users/me")
      |> Js.Promise.catch(err => {
        Js.log(err);

        Js.Promise.resolve({ "id": "" });
      }),
  },
  "ticket": {
    "create": (json) => post("/tickets/", json |> encode),
    "get": (ticketId) => get({j|/tickets/$ticketId|j}),
    "delete": (ticketId) => delete({j|/tickets/$ticketId|j}),
  },
  "comment": {
    "list": (ticketId) => get({j|/tickets/$ticketId/comments|j}),
  },
  "page": {
    "list": (ticketId) => get({j|/tickets/$ticketId/pages|j}),
  },
  "project": {
    "create": (json) => post("/projects/", json |> encode),
    "listRecent": () => get("/projects/recent"),
  },
};
