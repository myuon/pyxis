<template>
  <div id="app">
    <el-container>
      <el-aside class="sidebar">
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
              Login as Guest
              <button class="action" type="mini">Sign in</button>
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
      </el-aside>

      <el-main class="main">
        <router-view/>
      </el-main>
    </el-container>
  </div>
</template>

<script>
export default {
  components: {
  },
  data () {
    return {
      activeItem: 'Home',
    };
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
  width: 300px;
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
