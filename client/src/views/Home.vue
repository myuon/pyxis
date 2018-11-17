<template>
  <div class="home">
    <div :key="project.id" v-for="project in recent" style="margin-bottom: 1rem;">
      <h3>{{ project.title }}</h3>

      <el-table
        :data="project.tickets"
        style="margin-bottom: 1rem;"
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

      <el-button type="default" icon="el-icon-plus" size="small">New Ticket</el-button>
    </div>

    <div>
      <el-button type="success" icon="el-icon-plus" @click="newProjectDialog = true">New Project</el-button>

      <el-dialog
        title="Create New Project"
        :visible.sync="newProjectDialog"
        width="40%"
        :before-close="handleClose">
        <el-form :model="projectForm" label-width="120px">
          <el-form-item label="Project Name">
            <el-input placeholder="Project Name" v-model="projectForm.title"></el-input>
          </el-form-item>
        </el-form>
        
        <span slot="footer" class="dialog-footer">
          <el-button @click="newProjectDialog = false">Cancel</el-button>
          <el-button type="primary" @click="createProject" :disabled="projectForm.title == ''">Confirm</el-button>
        </span>
      </el-dialog>
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
      newProjectDialog: false,
      projectForm: {
        title: '',
      }
    };
  },
  methods: {
    handleClose: async function(done) {
      await this.$confirm('Are you sure to close this dialog?');
      done();
    },
    createProject: async function() {
      const res = await client.project.create({
        title: this.projectForm.title,
        owned_by: "1",
      });
      console.log(res);
      await this.loadRecentProjects();
      this.newProjectDialog = false;

      this.projectForm.title = '';
    },
    loadRecentProjects: async function() {
      this.recent = await client.project.listRecent();
    },
  },
  mounted: async function () {
    await this.loadRecentProjects();
  }
}
</script>

<style scoped>
.footer {
  margin-top: 1em;
}
</style>
