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
            @success="onSignUpSuccess"
            @error="onSignUpError">
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
import token from '@/token';
import { client } from '@/client.bs';

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
    onSignUpSuccess (googleUser) {
      client.auth.signIn({
        userName: 'myuon',
        email: 'ioi.joi.koi.loi@gmail.com',
        idp: 'google',
        token: googleUser.getAuthResponse().id_token,
      });
    },
    onSignUpError (error) {
      console.log(error);
    },
    onSignInSuccess: async function (googleUser) {
      console.log({
        token: googleUser.getAuthResponse().id_token,
      });
      const result = await client.auth.signIn({
        token: googleUser.getAuthResponse().id_token,
      });
      console.log(result.token);
    },
    onSignInError (error) {
      console.log(error);
    },
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

