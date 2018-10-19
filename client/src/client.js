import axios from 'axios';
import lib from 'pyxis-lib';

const endpoint = 'http://localhost:3000';

export const Client = {
  user: {
    me: async () => {
      return lib.validate(lib.user, (await axios.get(`${endpoint}/users/me`)).data);
    },
  },
  project: {
    list_recent: async () => {
      return lib.validate({ type: 'array', items: lib.project }, (await axios.get(`${endpoint}/projects/recent`)).data);
    },
  },
  ticket: {
    get: async (projectId, ticketId) => {
      const ticket = lib.validate(lib.ticket, (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}`)).data);
      const pages = lib.validate({ type: 'array', items: lib.page }, (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}/pages`)).data);
      const comments = lib.validate({ type: 'array', items: lib.comment }, (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}/comments`)).data);
      ticket.comments = comments;
      ticket.pages = pages;
      return ticket;
    },
  },
};

