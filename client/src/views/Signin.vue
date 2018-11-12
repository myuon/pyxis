<template>
  <amplify-authenticator class="center" v-bind:authConfig="authConfig" />
</template>

<script>
import Amplify, * as AmplifyModules from 'aws-amplify';
import { AmplifyPlugin, AmplifyEventBus } from 'aws-amplify-vue';
import aws_exports from '@/aws-exports';
import Vue from 'vue';
Amplify.configure(aws_exports);
Vue.use(AmplifyPlugin, AmplifyModules);

export default {
  name: 'login',
  components: {
  },
  data () {
    return {
      authConfig: {},
    };
  },
  mounted: async function () {
    AmplifyEventBus.$on('authState', (state) => {
      if (state === 'signedIn') {
        this.$router.push('/');
      }
    });
    
    await this.$Amplify.Auth.currentUserCredentials();
    if (this.$Amplify.user != null) {
      this.$router.push('/');
    }
  },
}
</script>

<style scoped>
.center {
  position: absolute;
  top: 30px;
  left: 0;
  right: 0;
  width: 460px;
  margin: auto;
}
</style>

