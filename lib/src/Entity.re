module Project = {
  type t = {
    id: string,
    title: string,
    tickets: array(string),
  };

  let decode : Js.Json.t => t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      title: json |> field("title", string),
      tickets: json |> field("tickets", array(string)),
    }
  };

  let encode : t => Js.Json.t = project => {
    Json.Encode.(
      object_([
        ("id", project.id |> string),
        ("title", project.title |> string),
        ("tickets", project.tickets |> array(string)),
      ])
    )
  };
};

module Comment = {
  type t = {
    id: string,
    content: string,
    created_at: Js.Date.t,
    owner: string,
    belongs_to: Js.Dict.t(string),
  };

  let decode : Js.Json.t => t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      content: json |> field("content", string),
      created_at: json |> field("created_at", string) |> Js.Date.fromString,
      owner: json |> field("owner", string),
      belongs_to: json |> field("belongs_to", dict(string)),
    }
  };

  let encode : t => Js.Json.t = comment => {
    Json.Encode.(
      object_([
        ("id", comment.id |> string),
        ("content", comment.content |> string),
        ("created_at", comment.created_at |> Js.Date.toISOString |> string),
        ("owner", comment.owner |> string),
        ("belongs_to", comment.belongs_to |> Js.Dict.map((. value) => string(value)) |> dict),
      ])
    )
  };
};

module Page = {
  type t = {
    id: string,
    title: string,
    content: string,
    belongs_to: Js.Dict.t(string),
  }

  let decode : Js.Json.t => t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      title: json |> field("title", string),
      content: json |> field("content", string),
      belongs_to: json |> field("belongs_to", dict(string)),
    }
  };

  let encode : t => Js.Json.t = page => {
    Json.Encode.(
      object_([
        ("id", page.id |> string),
        ("title", page.title |> string),
        ("content", page.content |> string),
        ("belongs_to", page.belongs_to |> Js.Dict.map((. value) => string(value)) |> dict),
      ])
    )
  };
};

module Ticket = {
  type t = {
    id: string,
    title: string,
    assigned_to: array(string),
    comment: int,
    belongs_to: Js.Dict.t(string),
  };

  let decode : Js.Json.t => t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      title: json |> field("title", string),
      assigned_to: json |> field("assigned_to", array(string)),
      comment: json |> field("comment", int),
      belongs_to: json |> field("belongs_to", dict(string)),
    }
  };

  let encode : t => Js.Json.t = ticket => {
    Json.Encode.(
      object_([
        ("id", ticket.id |> string),
        ("title", ticket.title |> string),
        ("assinged_to", ticket.assigned_to |> array(string)),
        ("comment", ticket.comment |> int),
        ("belongs_to", ticket.belongs_to |> Js.Dict.map((. value) => string(value)) |> dict),
      ])
    )
  };
};

module User = {
  type t = {
    id: string,
    name: string,
    display_name: string,
    created_at: Js.Date.t,
  };

  let decode : Js.Json.t => t = json => {
    Json.Decode.{
      id: json |> field("id", string),
      name: json |> field("name", string),
      display_name: json |> field("display_name", string),
      created_at: json |> field("created_at", string) |> Js.Date.fromString,
    }
  };

  let encode : t => Js.Json.t = user => {
    Json.Encode.(
      object_([
        ("id", user.id |> string),
        ("name", user.name |> string),
        ("display_name", user.display_name |> string),
        ("created_at", user.created_at |> Js.Date.toISOString |> string),
      ])
    )
  };
};
