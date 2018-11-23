module QueryResult = {
  type one('t) = {
    item: 't,
  };

  type many('t) = {
    items: array('t),
    count: int,
    scannedCount: int,
  };

  let parseOne = (decoder, json) => {
    Json.Decode.{
      item: json |> field("Item", decoder),
    }
  };

  let parseMany = (decoder, json) => {
    Json.Decode.{
      items: json |> field("Items", array(decoder)),
      count: json |> field("Count", int),
      scannedCount: json |> field("ScannedCount", int),
    }
  };
};

module Project = {
  type t = Js.t({
    .
    id: string,
    title: string,
    tickets: array(string),
    owned_by: string,
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("project##") |> x => x[1],
      title: t##title,
      tickets: t##tickets,
      owned_by: t##owned_by,
    }]
  };

  external encode : 't => Js.Json.t = "%identity";

  let list = (userId) => {
    DAO.listByOwner(
      ~owner=userId,
      ~idPrefix="project",
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve
    })
  };

  let create : Js.t({. owned_by: string, title: string}) => Js.Promise.t(Js.Json.t) = (json) => {
    let id = UUID.uuid();

    DAO.create({
      "id": {j|project-$id|j},
      "sort": "detail",
      "tickets": [||],
      "title": json##title,
      "owned_by": json##owned_by,
    });
  };

  let update = (projectId, label, value) => {
    DAO.update(
      ~id={j|project-$projectId|j},
      ~sort="detail",
      ~label=label,
      ~value=value
    );
  };

  let get = (projectId) => {
    DAO.get(
      ~id={j|project-$projectId|j},
      ~sort="detail",
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let delete = (projectId) => {
    DAO.list(
      ~id={j|project-$projectId|j},
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(x => x)
      |> x => x.items
      |> Belt.Array.map(_, item => {
        {
          "id": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "id") |> Js.Json.decodeString |> Js.Option.getExn,
          "sort": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "sort") |> Js.Json.decodeString |> Js.Option.getExn,
        }        
      })
      |> DAO.batchDelete;
    });
  };
};

module Ticket = {
  type t = Js.t({
    .
    id: string,
    title: string,
    comment: int,
    assigned_to: array(string),
    belongs_to: Js.t({ .
      project: string,
    }),
    owned_by: string,
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("ticket##") |> x => x[1],
      title: t##title,
      comment: t##comment,
      assigned_to: t##assigned_to,
      belongs_to: {
        project: t##belongs_to##project,
      },
      owned_by: t##owned_by,
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

  let create : Js.t({
    .
    title: string,
    assigned_to: array(string),
    belongs_to: Js.t({
      .
      project: string,
    }),
    owned_by: string,
  }) => Js.Promise.t(string) = (json) => {
    let id = UUID.uuid();

    DAO.create({
      "id": {j|ticket##$id|j},
      "sort": "detail",
      "title": json##title,
      "comment": 0,
      "assigned_to": json##assigned_to,
      "belongs_to": {
        "project": json##belongs_to##project,
      },
      "owned_by": json##owned_by,
    })
    |> Js.Promise.then_(_ => {
      id
      |> Js.Promise.resolve;
    });
  };

  let list = (userId) => {
    DAO.listByOwner(
      ~owner=userId,
      ~idPrefix="ticket",
      ~sort="detail",
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve;
    })
  };

  let get = (ticketId) => {
    DAO.get(
      ~id={j|ticket##$ticketId|j},
      ~sort="detail",
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let update = (ticketId, label, value) => {
    DAO.update(
      ~id={j|ticket##$ticketId|j},
      ~sort="detail",
      ~label=label,
      ~value=value
    );
  };

  let delete = (ticketId) => {
    DAO.list(
      ~id={j|ticket##$ticketId|j},
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(x => x)
      |> x => x.items
      |> Belt.Array.map(_, item => {
        {
          "id": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "id") |> Js.Json.decodeString |> Js.Option.getExn,
          "sort": item |> Js.Json.decodeObject |> Js.Option.getExn |> Js.Dict.unsafeGet(_, "sort") |> Js.Json.decodeString |> Js.Option.getExn,
        }
      })
      |> DAO.batchDelete;
    });
  };
};

module Page = {
  type t = Js.t({
    .
    id: string,
    title: string,
    content: string,
    belongs_to: Js.t({
      .
      project: string,
      ticket: string,
    }),
    owned_by: string,
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("ticket##") |> x => x[1],
      title: t##title,
      content: t##content,
      belongs_to: {
        project: t##belongs_to##project,
        ticket: t##belongs_to##ticket,
      },
      owned_by: t##owned_by,
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

  let list = (ticketId) => {
    DAO.list(
      ~id={j|ticket##$ticketId|j},
      ~sortPrefix="page",
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve
    })
  };

  let create : Js.t({
    .
    id: string,
    title: string,
    content: string,
    belongs_to: Js.t({
      .
      project: string,
      ticket: string,
    }),
    owned_by: string,
  }) => Js.Promise.t(Js.Json.t) = (json) => {
    let ticketId = json##belongs_to##ticket;
    let pageId = json##id;

    DAO.create({
      "id": {j|ticket##$ticketId|j},
      "sort": {j|page##$pageId|j},
      "title": json##title,
      "content": if (json##content != "") { json##content } else { [%raw {| null |}] },
      "belongs_to": {
        "project": json##belongs_to##project,
      },
      "owned_by": json##owned_by,
    });
  };
};

module Comment = {
  type t = Js.t({
    .
    id: string,
    sort: string,
    content: string,
    created_at: string,
    owned_by: string,
    belongs_to: Js.t({
      .
      ticket: string,
      project: string,
    }),
  });

  external parse_ : Js.Json.t => t = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##sort |> Js.String.split("comment##", _) |> x => x[1],
      sort: t##sort,
      content: t##content,
      created_at: t##created_at,
      owned_by: t##owned_by,
      belongs_to: {
        ticket: t##belongs_to##ticket,
        project: t##belongs_to##project,
      },
    }]
  };

  external encode : 'a => Js.Json.t = "%identity";

  let list = (ticketId) => {
    DAO.list(
      ~id={j|ticket##$ticketId|j},
      ~sortPrefix="comment",
      ()
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseMany(parse)
      |> Js.Promise.resolve;
    })
  };

  let create : (string, {
    .
    "content": string,
    "belongs_to": {
      .
      "project": string,
      "ticket": string,
    },
    "owned_by": string,
  }) => Js.Promise.t(Js.Json.t) = (commentId, item) => {
    let ticketId = item##belongs_to##ticket;

    DAO.create({
      "id": {j|ticket##$ticketId|j},
      "sort": {j|comment##$commentId|j},
      "content": item##content,
      "belongs_to": item##belongs_to,
      "created_at": Js.Date.make() |> Js.Date.toISOString,
      "owned_by": item##owned_by,
    });
  };
};

module Idp = {
  type t = Js.t({
    .
    id: string,
    idp: string,
    owned_by: string,
  });

  external parse_ : Js.Json.t => 'a = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    /* Assume that idpId does not include hyphens: this is unsafe a bit */
    let [| _, idp, idpId |] = t##id |> Js.String.split("##");

    [%bs.obj {
      id: idpId,
      idp: idp,
      owned_by: t##owned_by,
    }]
  };
};

module User = {
  type t = Js.t({
    .
    id: string,
    sort: string,
    idp: Js.t({
      .
      google: string,
    }),
    created_at: string,
    user_name: string,
  });

  external encode : 'a => Js.Json.t = "%identity";
  external parse_ : Js.Json.t => 'a = "%identity";
  let parse : Js.Json.t => t = json => {
    let t = parse_(json);

    [%bs.obj {
      id: t##id |> Js.String.split("user##", _) |> x => x[1],
      sort: t##sort,
      idp: {
        google: t##idp##google,
      },
      created_at: t##created_at,
      user_name: t##user_name,
    }]
  };

  let findById = userId => {
    DAO.get(
      ~id={j|user##$userId|j},
      ~sort="detail",
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let findViaIdp = (idp, idp_id) => {
    DAO.get(
      ~id={j|idp##$idp##$idp_id|j},
      ~sort="detail",
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(Idp.parse)
      |> Js.Promise.resolve;
    });
  };

  let createIdp = (idp, idpId, userId) => {
    DAO.create({
      "id": {j|idp##$idp##$idpId|j},
      "sort": "detail",
      "owned_by": userId
    });
  };

  let create = (idp, userName) => {
    let userId = UUID.uuid();
    
    DAO.create({
      "id": {j|user##$userId|j},
      "sort": "detail",
      "idp": idp,
      "created_at": Js.Date.make() |> Js.Date.toISOString,
      "user_name": userName,
    })
    |> Js.Promise.then_(_ => {
      userId
      |> Js.Promise.resolve;
    });
  };

  let get = (userId) => {
    DAO.get(
      ~id={j|user##$userId|j},
      ~sort="detail",
    )
    |> Js.Promise.then_(result => {
      result
      |> QueryResult.parseOne(parse)
      |> Js.Promise.resolve;
    });
  };

  let getByName = (userName) => {
    {
      "TableName": "userId",
      "Key": {
        "user_name": userName,
      },
    }
    |> encode
    |> DAO.DBC.get(DAO.DBC.dbc,_)
    |> DAO.DBC.promise;
  };

  let createNameAlias = (userName, userId) => {
    {
      "TableName": "userId",
      "Item": {
        "user_name": userName,
        "id": userId,
      },
    }
    |> encode
    |> DAO.DBC.put(DAO.DBC.dbc,_)
    |> DAO.DBC.promise;
  };
};
