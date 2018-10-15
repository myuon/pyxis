<template>
  <div class="home">
    <div :key="project.id" v-for="project in client.project.list_recent()">
      <h3>{{ project.title }}</h3>

      <el-card class="box-card" v-for="ticket in project.tickets">
        <div slot="header" class="clearfix">
          <span><strong>#3028</strong> Something happened!</span>
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
          <el-button @click="$router.push(`/ticket/${ticket}`)" type="primary" size="small">Open</el-button>
        </div>
      </el-card>
    </div>
  </div>
</template>

<script>
import { Client } from '@/client';

export default {
  name: 'home',
  components: {
  },
  data () {
    return {
      client: Client,
    };
  },
}
</script>

<style scoped>
.footer {
  margin-top: 1em;
}
</style>
