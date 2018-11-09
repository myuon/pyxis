<template>
  <div class="container">
    <el-card class="box-card">
      <div slot="header" class="clearfix">
        <span v-if="ticket.id != null">{{ ticket.belongs_to.project }} > #{{ ticket.id }}</span>
        <el-button @click="$router.push('/')" style="float: right; padding: 3px 0" type="text">Back to List</el-button>
      </div>

      <h2>{{ ticket.title }}</h2>

      <el-collapse v-model="active_tabs" class="ticket-data">
        <el-collapse-item name="1">
          <template slot="title">
            <span class="collapse-title">Ticket Data</span>
          </template>

          <div class="form-control">
            <span>Ticket number</span>
            <el-input
              disabled
              v-model="ticket.id">
            </el-input>
          </div>

          <div class="form-control">
            <span>Assigned to</span>

            <el-select
              v-model="ticket.assigned_to"
              multiple
              collapse-tags
              placeholder="Select"
              style="width: 100%;">
              <el-option
                v-for="item in members"
                :key="item.value"
                :label="item.label"
                :value="item.value">
              </el-option>
            </el-select>
          </div>

          <div class="form-control">
            <span>Deadline</span>

            <el-date-picker
              v-model="ticket.deadline"
              type="date"
              placeholder="Pick a day"
              style="width: 100%;">
            </el-date-picker>
          </div>

        </el-collapse-item>

        <el-collapse-item name="2">
          <template slot="title">
            <span class="collapse-title">Content</span>
          </template>

          <el-tabs v-model="currentPage">
            <el-tab-pane :label="page.title" :key="index" :name="index.toString()" v-for="(page, index) in ticket.pages">
              <div class="float">
                <el-button-group class="markdown-mode">
                  <el-button @click="setMode('view')" :class="{ current: isView }" type="default" size="mini"><i class="material-icons">visibility</i></el-button>
                  <el-button @click="setMode('edit')" :class="{ current: isEdit }" type="default" size="mini"><i class="material-icons">edit</i></el-button>
                </el-button-group>
              </div>

              <el-input
                type="textarea"
                :rows="15"
                placeholder="Please input"
                v-model="page.content"
                v-if="md_mode === 'edit'"
              />
              <vue-markdown
                :source="page.content"
                class="markdown-view"
                v-if="md_mode === 'view'"
              />
            </el-tab-pane>
          </el-tabs>
        </el-collapse-item>

        <el-collapse-item name="3">
          <template slot="title">
            <span class="collapse-title">Comments</span>
          </template>

          <el-row class="comment-feed" justify="space-between" :key="comment.id" v-for="comment in comments">
            <div class="meta">
              <span class="timestamp">#{{ comment.id }} [{{ comment.created_at }}]</span>
            </div>

            <el-col :span="1" class="icon">
              <i class="material-icons">person</i>
            </el-col>

            <el-col :offset="1" :span="11">
              <div class="comment">
                <span class="summary">@{{ comment.owner }} commented:</span>
                <vue-markdown :source="comment.content" />
              </div>
            </el-col>
          </el-row>

          <el-row>
            <el-col :span="1" class="icon">
              <i class="material-icons">person</i>
            </el-col>

            <el-col :offset="1" :span="11">
              <div class="comment">
                <el-input
                  type="textarea"
                  :rows="3"
                  placeholder="Please input"
                  v-model="newCommentText"
                />

                <el-button @click="submitComment" type="success" size="small">Submit</el-button>
              </div>
            </el-col>
          </el-row>
        </el-collapse-item>

      </el-collapse>
    </el-card>
  </div>
</template>

<script>
import { Client } from '@/client';
import VueMarkdown from 'vue-markdown';

export default {
  name: 'ticket',
  components: {
    VueMarkdown,
  },
  computed: {
    isView () {
      return this.md_mode === 'view';
    },
    isEdit () {
      return this.md_mode === 'edit';
    },
  },
  data () {
    return {
      members: [
        { value: '1', label: 'わたし' },
        { value: '2', label: 'あなた' },
        { value: '3', label: 'かれ' },
      ],
      currentPage: '0',
      md_mode: 'view',
      active_tabs: [ '1', '2', '3' ],
      ticket: {},
      comments: [],
      newCommentText: '',
    }
  },
  methods: {
    setMode (mode) {
      this.md_mode = mode;
    },
    submitComment: async function () {
      await Client.Comment.create(this.ticket.belongs_to.project, this.$route.params.ticketId, this.newCommentText);
      this.newCommentText = '';
      await this.loadComments();
    },
    loadComments: async function () {
      this.comments = await Client.comment.list(this.$route.params.ticketId);
    },
  },
  mounted: async function () {
    this.ticket = await Client.ticket.get(this.$route.params.ticketId);
    await this.loadComments();
  },
}
</script>

<style scoped>
.container {
  margin-left: 1em;
  width: 80%;
  box-shadow: 0 0 20px 0 rgba(0,0,0,0.05);
}

h2 {
  font-family: 'Lato', 'Noto Sans JP', sans-serif;
}

.collapse-title {
  font-size: larger;
  font-weight: bold;
}

.ticket-title {
  margin-top: 30px;
  padding-top: 10px;
}

.ticket-data .form-control {
  margin-top: 10px;
  width: 40%;
}

.float {
  position: relative;
}

.markdown-mode {
  position: absolute;
  top: 0px;
  right: 0px;
}

.markdown-mode .el-button {
  padding: 5px 10px;
}

.markdown-mode .material-icons {
  font-size: 18px;
}

.markdown-mode .current {
  background-color: #9bd;
  color: #fff;
}

.markdown-view {
  padding-top: 1px;
  padding-bottom: 10px;
  padding-left: 10px;
  background-color: #eee;
}

.icon i {
  font-size: 36px;
  padding: 6px;
  background-color: #ccc;
  border-radius: 60px;
}

.comment-feed .meta .timestamp {
  color: #666;
  border-bottom: 1px solid #aaa;
  padding-bottom: 2px;
}

.comment-feed .meta {
  margin-bottom: 10px;
}

.comment-feed .comment .summary {
  font-weight: bold;
}
</style>
