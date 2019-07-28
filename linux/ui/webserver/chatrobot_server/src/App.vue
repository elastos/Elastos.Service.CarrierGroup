<template>
 <a-layout id="components-layout">
    <a-layout-header></a-layout-header>
      <a-layout>
        <a-layout-content><GroupList/></a-layout-content>
        <a-layout-sider>
            <a-card
             hoverable
             >

             <a-card-meta
               title="群：">
               <template slot="description">1</template>
             </a-card-meta>
        </a-card></a-layout-sider>
      </a-layout>
     <a-layout-footer></a-layout-footer>
    </a-layout>
</template>
<script>
import GroupList from './components/GroupList.vue'
export default {
  name: 'app',
  components: {
    GroupList,
  },
  data(){
      return {
          config: {
            value: 'www.baidu.com',//显示的值、跳转的地址
          },
        collapsed: false,
      }
    }
}
</script>
<style>
#components-layout {
  text-align: center;
}
#components-layout .ant-layout-header,
#components-layout .ant-layout-footer {
  background: #7dbcea;
  color: #ff0;
}
#
#components-layout .ant-layout-content {
  background: rgba(16, 142, 233, 1);
  color: #fff;
  min-height: 700px;
  line-height: 120px;
}

#components-layout .ant-layout-sider{
background: #f0f2f5;
  vertical-align: middle;
  left:-300px;
  width:300px;
  max-width:500px;
}
#components-layout .ant-card {
background: #7dbcea;

  width:400px;
  height:400px;
  max-width:500px;
}
#components-layout > .ant-layout {
  margin-bottom: 48px;
}
#components-layout > .ant-layout:last-child {
  margin: 0;
}
</style>