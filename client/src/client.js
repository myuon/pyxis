import axios from 'axios';

const endpoint = 'http://localhost:3000';

export const Client = {
  user: {
    me: () => {
      return axios.get(`${endpoint}/users/me`).then(res => res.data);
    },
  },
  project: {
    list_recent: () => {
      return axios.get(`${endpoint}/projects/recent`).then(res => res.data);
    },
  },
  ticket: {
    get: async (projectId, ticketId) => {
      const ticket = (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}`)).data;
      const pages = (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}/pages`)).data;
      const comments = (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}/comments`)).data;
      ticket.comments = comments;
      ticket.pages = pages;
      return ticket;
    },
  },
};

