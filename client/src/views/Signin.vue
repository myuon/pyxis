<template>
  <div class="center">
    <el-card class="box-card">
      <el-tabs v-model="activeTab">
        <el-tab-pane label="Sign In" name="signIn">
          <g-signin-button
            :params="googleSignInParams"
            @success="onSignInSuccess"
            @error="onSignInError">
            <el-button type="primary">
              Sign in with Google
            </el-button>
          </g-signin-button>
        </el-tab-pane>
        <el-tab-pane label="Sign Up" name="signUp">
          <g-signin-button
            :params="googleSignInParams"
            @success="onSignInSuccess"
            @error="onSignInError">
            <el-button type="primary">
              Sign up with Google
            </el-button>
          </g-signin-button>
        </el-tab-pane>
      </el-tabs>
    </el-card>
  </div>
</template>

<script>
import Amplify, * as AmplifyModules from 'aws-amplify';
import { AmplifyPlugin, AmplifyEventBus } from 'aws-amplify-vue';
import aws_exports from '@/aws-exports';
import Vue from 'vue';
Amplify.configure(aws_exports);
Vue.use(AmplifyPlugin, AmplifyModules);

import token from '@/token';

export default {
  name: 'login',
  components: {
  },
  data () {
    return {
      activeTab: 'signIn',
      authConfig: {},
      googleSignInParams: {
        client_id: token.google.clientId,
      },
    };
  },
  methods: {
    onSignInSuccess (googleUser) {
      console.log(googleUser.getBasicProfile().getName());
    },
    onSignInError (error) {
      console.log(error);
    },
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

