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
          <el-form ref="form" :rules="formRules" :model="form" label-width="100px" status-icon>
            <el-form-item label="User Name" prop="user_name">
              <el-input size="small" v-model="form.user_name" autocomplete="off">
                <template slot="prepend">@</template>
              </el-input>
            </el-form-item>
            <el-form-item>
              <g-signin-button
                :params="googleSignInParams"
                @success="onSignUpSuccess"
                @error="onSignUpError">
                <el-button type="primary">
                  Sign up with Google
                </el-button>
              </g-signin-button>
            </el-form-item>
          </el-form>
        </el-tab-pane>
      </el-tabs>
    </el-card>
  </div>
</template>

<script>
import token from '../../../token/google.json';
import { client } from '@/client.bs';

export default {
  name: 'login',
  components: {
  },
  data () {
    const checkIsAvailable = async (rule, value, callback) => {
      if (value === '') {
        callback(new Error('Please input the user name'));
      } else {
        const result = await client.auth.userNameAvailable(this.form.user_name);

        if (!result) {
          callback(new Error('This user name is not available'));
        }

        callback();
      }
    };

    return {
      activeTab: 'signIn',
      authConfig: {},
      googleSignInParams: {
        client_id: token.web.client_id,
      },
      form: {
        user_name: ''
      },
      formRules: {
        user_name: [
          { validator: checkIsAvailable, trigger: 'change' }
        ]
      },
    };
  },
  methods: {
    onSignUpSuccess: async function (googleUser) {
      const result = await client.auth.signUp({
        user_name: this.form.user_name,
        token: googleUser.getAuthResponse().id_token,
      });
      if (result.result === true) {
        await this.onSignInSuccess(googleUser);
      }
    },
    onSignUpError (error) {
      console.log(error);
    },
    onSignInSuccess: async function (googleUser) {
      await client.auth.signIn({
        token: googleUser.getAuthResponse().id_token,
      });
      this.$router.push('/');
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
  width: 480px;
  margin: auto;
}
</style>

