<template>
  <div class="home">
    <div :key="project.id" v-for="project in recent">
      <h3>{{ project.title }}</h3>

      <el-table
        :data="project.tickets"
      >
        <el-table-column
          prop="id"
          label="id"
        />
        <el-table-column
          prop="title"
          label="title"
        />
        <el-table-column
          prop="comment"
          label="comment"
        />
        <el-table-column
          prop="id"
        >
          <template slot-scope="scope">
            <el-button @click="$router.push(`/tickets/${scope.row.id}`)" type="primary" size="mini">Open</el-button>
          </template>
        </el-table-column>
      </el-table>
    </div>
  </div>
</template>

<script>
import { client } from '@/client.bs';

export default {
  name: 'home',
  data () {
    return {
      recent: [],
    };
  },
  mounted: async function () {
    this.recent = await client.project.listRecent();
  }
}
</script>

<style scoped>
.footer {
  margin-top: 1em;
}
</style>
