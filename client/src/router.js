import Vue from 'vue';
import Router from 'vue-router';
import Home from './views/Home.vue';
import Ticket from './views/Ticket.vue';
import Signin from './views/Signin.vue';
import SideBar from './views/SideBar.vue';

Vue.use(Router)

export default new Router({
  mode: 'history',
  base: process.env.BASE_URL,
  routes: [
    {
      path: '/',
      name: 'home',
      components: {
        default: Home,
        sidebar: SideBar,
      }
    },
    {
      path: '/tickets/:ticketId',
      name: 'ticket',
      components: {
        default: Ticket,
        sidebar: SideBar,
      },
    },
    {
      path: '/signin',
      name: 'signin',
      components: {
        default: Signin,
      },
    },
  ]
})
