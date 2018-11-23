<template>
  <el-menu
    default-active="2"
    class="side-menu"
    background-color="#545c64"
    text-color="#fff"
    active-text-color="#ffd04b">
    <h2 class="logo">
      <i class="material-icons">my_location</i>
      <router-link to="/">Pyxis</router-link>
    </h2>

    <div class="login-state">
      <i class="material-icons">person_pin</i>
      <div class="user">
        Login as {{ user.user_name }}
        <button @click="signOut" class="action" type="mini">Sign out</button>
      </div>
    </div>

    <el-submenu index="1">
      <template slot="title">
        <i class="el-icon-location"></i>
        <span>Documents</span>
      </template>
      <el-menu-item-group title="Group One">
        <el-menu-item index="1-1">item one</el-menu-item>
        <el-menu-item index="1-2">item one</el-menu-item>
      </el-menu-item-group>
      <el-menu-item-group title="Group Two">
        <el-menu-item index="1-3">item three</el-menu-item>
      </el-menu-item-group>
      <el-submenu index="1-4">
        <template slot="title">item four</template>
        <el-menu-item index="1-4-1">item one</el-menu-item>
      </el-submenu>
    </el-submenu>
    <el-menu-item index="2">
      <i class="el-icon-menu"></i>
      <span>Navigator Two</span>
    </el-menu-item>
    <el-menu-item index="3" disabled>
      <i class="el-icon-document"></i>
      <span>Navigator Three</span>
    </el-menu-item>
    <el-menu-item index="4">
      <i class="el-icon-setting"></i>
      <span>Navigator Four</span>
    </el-menu-item>
  </el-menu>
</template>

<script>
import { client } from '@/client.bs';

export default {
  name: 'side-bar',
  data () {
    return {
      user: {},
    };
  },
  mounted: async function () {
    this.user = await client.user.me();
    console.log(this.uesr);

    if (this.user.message === 'Unauthorized') {
      this.$router.push('/signIn');
    }
  },
  methods: {
    signOut: async function () {
      this.user = null;

      // clear jwt token
      document.cookie = 'token=; expires=Thu, 01 Jan 1970 00:00:01 GMT;';

      this.$router.push('/signin');
    },
  },
}
</script>
