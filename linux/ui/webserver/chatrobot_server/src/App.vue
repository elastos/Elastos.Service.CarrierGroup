<template>
 <a-layout id="components-layout">
    <a-layout-header></a-layout-header>
      <a-layout>
          <a-layout-sider>
              <a-button type="primary" @click="createGroup">CreateGroup</a-button>
          </a-layout-sider>
        <a-layout-content><GroupList/></a-layout-content>
      </a-layout>
     <a-layout-footer></a-layout-footer>
    </a-layout>
</template>
<script>
import GroupList from './components/GroupList.vue'
import reqwest from 'reqwest'

export default {
  name: 'app',
  components: {
    GroupList,
  },
  data(){
      return {
        collapsed: false,
      }
    },
    methods:{
        createGroup() {
            window.console.log("createGroup");
            reqwest({
                    url:window.location.href.replace("8080","8081")+"/create",
                    type: 'json',
                    method: 'get',
                    contentType: 'application/json',
                    success: () => {
                      //刷新列表
                    },
                  })
        }
    }
}
</script>

<style>
#components-layout {
  text-align: left;
}
#components-layout .ant-layout-header,
#components-layout .ant-layout-footer {
  color: #ff0;
}

#components-layout .ant-layout-content {
  color: #fff;
  min-height: 700px;
  line-height: 120px;
}

#components-layout .ant-layout-sider{
    width:350px;
    height:600px;
}

#components-layout > .ant-layout {
  margin-bottom: 48px;
}
#components-layout > .ant-layout:last-child {
  margin: 0;
}
</style>
