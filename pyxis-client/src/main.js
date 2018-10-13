import Vue from 'vue';
import router from './router';

import ElementUI from 'element-ui';
import 'element-ui/lib/theme-chalk/index.css';
Vue.use(ElementUI);

import 'material-icons/iconfont/material-icons.css'

import VueMarkdown from 'vue-markdown'
Vue.use(VueMarkdown)

import App from './App.vue'

Vue.config.productionTip = false

new Vue({
  router,
  render: h => h(App),
}).$mount('#app')
