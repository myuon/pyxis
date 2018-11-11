<template>
  <el-row>
    <el-col :offset="6" :span="8">
      <el-card>
        <el-form>
          <el-form-item label="User ID">
            <el-input v-model="form.userId" />
          </el-form-item>
          <el-form-item label="Password">
            <el-input v-model="form.password" />
          </el-form-item>
          <el-button @click="$router.push('/')" type="primary">Login</el-button>
        </el-form>
      </el-card>
    </el-col>
  </el-row>
</template>

<script>
export default {
  name: 'login',
  data () {
    return {
      form: {
        userId: '',
        password: '',
      },
    };
  },
}
</script>
