<template>
  <div id="app">
    <el-container>
      <el-aside class="sidebar">
        <router-view name="sidebar" />
      </el-aside>
      <el-main class="main">
        <router-view />
      </el-main>
    </el-container>
  </div>
</template>

<script>
import { client } from '@/client.bs';

export default {
  data () {
    return {
      activeItem: 'Home',
    };
  },
  mounted: async function () {
  },
}
</script>

<style>
@import url('https://fonts.googleapis.com/css?family=Lato|Noto+Sans+JP|Changa|Michroma');

body {
  font-family: 'Lato', 'Noto Sans JP', sans-serif;
  margin: 0;
  height: 100%;
}

.sidebar {
  max-width: 300px;
  position: fixed;
}

.main {
  margin-left: 300px;
}

.logo > i {
  color: #ffd04b;
  margin-right: 0.3em;
  font-size: 20px;
}

.logo {
  margin-left: 0.5em;
  font-family: 'Michroma', sans-serif;
  font-size: 28px;
}

.logo > a {
  color: #fff;
  text-decoration: none;
}

.login-state {
  color: #fff;
  margin: 15px 20px;
}

.login-state > i {
  font-size: 36px;
  border: 3px solid #fff;
  border-radius: 50%;
  padding: 10px;
}

.login-state > .user {
  display: inline-block;
  vertical-align: top;
  margin-left: 1em;
}

.login-state > .user > button {
  display: block;
  margin-top: 1em;
  background-color: transparent;
  color: #fff;
  padding: 5px 10px;
  border: 1px solid #fff;
  border-radius: 3px;
}

.login-state > .user > button:hover {
  background-color: rgba(255, 255, 255, 0.4);
}

.side-menu {
  height: 100vh;
  background-color: #545c64;
}

.menu-right {
  text-align: right;
}
</style>
