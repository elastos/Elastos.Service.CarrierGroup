<template>
 <a-layout id="components-layout">
    <a-layout-header></a-layout-header>
      <a-layout>
        <a-layout-content><GroupList/></a-layout-content>

          <a-side>
                <a-card
                  hoverable
                  >
                  <div>dadaf</div>
                  <a-card-meta
                    title="群：">
                    <template slot="description">1</template>
                  </a-card-meta>
                </a-card>
              </a-layout>
      </a-layout>
     <a-layout-footer></a-layout-footer>
    </a-layout>
</template>
<script>

</script>
<script>
import GroupList from './components/GroupList.vue'

export default {
  name: 'app',
  components: {
    GroupList
  },
  data(){
      return {
        collapsed: false,
      }
    },
}
</script>
<style>
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
  max-width:500px;
  min-height: 700px;
  line-height: 120px;
}

#ant-layout-sider {
  background:#7dbcea;
  color: #fff;

  max-width:400px;
}
#components-layout > .ant-layout {
  margin-bottom: 48px;
}
#components-layout > .ant-layout:last-child {
  margin: 0;
}
</style>