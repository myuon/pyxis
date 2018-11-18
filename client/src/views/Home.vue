<template>
  <div class="home">
    <div :key="project.id" v-for="project in recent" style="margin-bottom: 1rem;">
      <h3>{{ project.title }} ({{ project.tickets.length }})</h3>

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

      <el-button type="default" icon="el-icon-plus" size="small" @click="newTicketDialog = true; selectingProject = project.id;">New Ticket</el-button>
      <el-button type="danger" icon="el-icon-delete" size="small" @click="deleteProject(project.id)">Delete</el-button>
    </div>

    <el-dialog
      title="Create New Ticket"
      :visible.sync="newTicketDialog"
      width="60%"
      :before-close="handleClose"
    >
      <el-form :model="projectForm" label-width="120px">
        <el-form-item label="Ticket Title">
          <el-input placeholder="Title" v-model="ticketForm.title"></el-input>
        </el-form-item>
        <el-form-item label="Assigned to">
          <el-select v-model="ticketForm.assigned_to" multiple placeholder="Select">
          </el-select>
        </el-form-item>
        <el-form-item label="Deadline">
          <el-date-picker
            v-model="ticketForm.deadline"
            type="date"
            placeholder="Pick a day"
          >
          </el-date-picker>
        </el-form-item>
      </el-form>

      <span slot="footer" class="dialog-footer">
        <el-button @click="newTicketDialog = false">Cancel</el-button>
        <el-button type="primary" @click="createTicket" :disabled="ticketForm.title == ''">Confirm</el-button>
      </span>
    </el-dialog>

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
      },
      newTicketDialog: false,
      selectingProject: '',
      ticketForm: {
        title: '',
        assigned_to: [],
        deadline: null,
      },
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
    deleteProject: async function(id) {
      await client.project.remove(id);
      await this.loadRecentProjects();
    },
    loadRecentProjects: async function() {
      this.recent = await client.project.listRecent();
      console.log(this.recent);
    },
    createTicket: async function() {
      const res = await client.ticket.create({
        title: this.ticketForm.title,
        assigned_to: this.ticketForm.assigned_to,
        deadline: this.ticketForm.deadline,
        belongs_to: {
          project: this.selectingProject,
        },
        owned_by: "1",
      });
      await this.loadRecentProjects();
      this.newTicketDialog = false;

      this.ticketForm = {
        title: '',
        assigned_to: [],
        deadline: null,
      };
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
