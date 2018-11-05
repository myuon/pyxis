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

type user = {
  id: string,
  name: string,
  display_name: string,
  created_at: Js.Date.t,
};
