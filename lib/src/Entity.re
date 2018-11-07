type project = {
  id: string,
  title: string,
  tickets: array(string),
};

type ticket = {
  id: string,
  title: string,
  assigned_to: array(string),
  comment: int,
  belongs_to: Belt.Map.String.t(string),
};

type comment = {
  id: string,
  content: string,
  created_at: Js.Date.t,
  owner: string,
  belongs_to: Belt.Map.String.t(string),
};

type page = {
  id: string,
  title: string,
  content: string,
  belongs_to: Belt.Map.String.t(string),
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
        ("created_at", user.created_at |> Js.Date.toString |> string),
      ])
    )
  };
};
