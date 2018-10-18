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
    get: async (projectId, ticketId) => {
      const ticket = (await axios.get(`${endpoint}/project/${projectId}/ticket/${ticketId}`)).data;
      const comment = (await axios.get(`${endpoint}/project/${projectId}/ticket/${ticketId}/comment`)).data;
      ticket.comment = comment;
      return ticket;
    },
  },
};

