import axios from 'axios';

const endpoint = 'http://localhost:3000';

export const Client = {
  user: {
    me: async () => {
      return (await axios.get(`${endpoint}/users/me`)).data;
    },
  },
  project: {
    listRecent: async () => {
      return (await axios.get(`${endpoint}/projects/recent`)).data;
    },
  },
  ticket: {
    get: async (ticketId) => {
      const ticket = (await axios.get(`${endpoint}/tickets/${ticketId}`)).data;
      const pages = (await axios.get(`${endpoint}/tickets/${ticketId}/pages`)).data;
      ticket.pages = pages;
      return ticket;
    },
  },
  comment: {
    list: async (ticketId) => {
      return (await axios.get(`${endpoint}/tickets/${ticketId}/comments`)).data;
    },
    create: async (projectId, ticketId, content) => {
      const comment = {
        id: '',
        content: content,
        created_at: new Date().toISOString(),
        owner: '1',
        belongs_to: {
          project: projectId,
          ticket: ticketId,
        },
      };

      await axios.post(`${endpoint}/comments`, comment);
    }
  }
};
