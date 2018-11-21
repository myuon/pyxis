let endpoint = [%bs.raw {| process.env.NODE_ENV === 'production' ? "http://localhost:3000" : "https://k1dt1bsvp1.execute-api.ap-northeast-1.amazonaws.com/dev/" |}];
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
    "signUp": {. "user_name": string, "token": string} => Js.Promise.t(Js.Json.t),
    "userNameAvailable": (string) => Js.Promise.t(bool),
  },
  "user": {
    .
    "me": unit => Js.Promise.t({
      .
      "id": string,
      "user_name": string,
      "created_at": string,
      "idp": {
        .
        "google": string,
      }
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
    "remove": string => Js.Promise.t(Js.Json.t),
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
    })),
    "create": {
      .
      "content": string,
      "belongs_to": {
        .
        "project": string,
        "ticket": string,
      },
      "owned_by": string,
    } => Js.Promise.t(Js.Json.t),
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
      "tickets": array(string),
      "owned_by": string,
    })),
    "remove": string => Js.Promise.t(Js.Json.t),
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
    },
    "signUp": (json) => post("/auth/signUp", json |> encode),
    "userNameAvailable": (userName) => get({j|/auth/user_name/is_available/$userName|j}),
  },
  "user": {
    "me": () => get("/users/me"),
  },
  "ticket": {
    "create": (json) => post("/tickets/", json |> encode),
    "get": (ticketId) => get({j|/tickets/$ticketId|j}),
    "remove": (ticketId) => delete({j|/tickets/$ticketId|j}),
  },
  "comment": {
    "list": (ticketId) => get({j|/tickets/$ticketId/comments|j}),
    "create": (json) => post("/comments/", json |> encode),
  },
  "page": {
    "list": (ticketId) => get({j|/tickets/$ticketId/pages|j}),
  },
  "project": {
    "create": (json) => post("/projects/", json |> encode),
    "listRecent": () => get("/projects/recent"),
    "remove": (projectId) => delete({j|/projects/$projectId|j}),
  },
};
