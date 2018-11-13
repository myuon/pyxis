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
      console.log(googleUser.getBasicProfile().getId());
      console.log(googleUser.getBasicProfile().getEmail());
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

