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
      ticket.pages = pages;
      return ticket;
    },
  },
  comment: {
    list: async (projectId, ticketId) => {
      return lib.validate({ type: 'array', items: lib.comment }, (await axios.get(`${endpoint}/projects/${projectId}/tickets/${ticketId}/comments`)).data);
    },
    create: async (projectId, ticketId, content) => {
      const comment = {
        id: '',
        content: content,
        created_at: (new Date()).toLocaleString('ja-JP'),
        owner: '1',
        belongs_to: '',
      };

      await axios.post(`${endpoint}/projects/${projectId}/tickets/${ticketId}/comments`, lib.validate(lib.comment, comment));
    }
  }
};
