<template>
  <div class="home">
    <el-card class="box-card">
      <div slot="header" class="clearfix">
        <span>#2018 Something happened!</span>
        <el-button style="float: right; padding: 3px 0" type="text">Operation button</el-button>
      </div>
      <div class="text item">
        Lorem ipsum dolor sit amet,
        consectetur adipiscing elit,
        sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.
        Ut enim ad minim veniam,
        quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.
      </div>

      <div class="footer">
        <el-button @click="$router.push('/ticket/1')" type="primary" size="small">Open</el-button>
      </div>
    </el-card>
  </div>
</template>

<script>
export default {
  name: 'home',
  components: {
  }
}
</script>

<style scoped>
.footer {
  margin-top: 1em;
}
</style>
