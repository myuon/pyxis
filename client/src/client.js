import axios from 'axios';

const endpoint = 'http://localhost:3000';

export const Client = {
  user: {
    me: () => {
      return axios.get(`${endpoint}/user/me`).then(res => res.data);
    },
  },
  project: {
    list_recent: () => {
      return axios.get(`${endpoint}/project/recent`).then(res => res.data);
    },
  },
  ticket: {
    get: (id) => {
      return {
        id: "abcd-ABCD",
        title: "some bug reported!",
        assigned_to: [ "1", "2" ],
        deadline: "2018-10-30",
      };
    },
  },
  comment: {
    get: (id) => {
      return {
        fetch: () => {
          return [
            {
              id: "1",
              content: "This is a sample comment.",
              created_by: "1",
              created_at: "2018-10-13 18:30:14",
            },
            {
              id: "2",
              content: "- hoge\n- piyo\n- nyan",
              created_by: "1",
              created_at: "2018-10-15 09:10:24",
            },
          ]
        },
      };
    },
  }
};

